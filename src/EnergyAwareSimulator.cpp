/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <memory>
#include <wrench-dev.h>

#include "EnergyAwareStandardJobScheduler.h"
#include "GreedyWMS.h"
#include "cost_model/TraditionalPowerModel.h"
#include "scheduling_algorithm/EnRealAlgorithm.h"
#include "scheduling_algorithm/IOAwareAlgorithm.h"
#include "scheduling_algorithm/IOAwareBalanceAlgorithm.h"
#include "scheduling_algorithm/SPSSEBAlgorithm.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(EnergyAwareSimulator, "Log category for EnergyAwareSimulator");

int main(int argc, char **argv) {
    // create and initialize the simulation
    wrench::Simulation simulation;
    simulation.init(&argc, argv);

    // check to make sure there are the right number of arguments
    if (argc < 3) {
        std::cerr << "WRENCH Pegasus WMS Simulator" << std::endl;
        std::cerr << "Usage: " << argv[0]
                  << " <xml platform file> <JSON workflow file>"
                  << std::endl;
        exit(1);
    }

    //create the platform file and dax file from command line args
    char *platform_file = argv[1];
    char *workflow_file = argv[2];

    // instantiating SimGrid platform
    WRENCH_INFO("Instantiating SimGrid platform from: %s", platform_file);
    simulation.instantiatePlatform(platform_file);

    // loading the workflow from the JSON file
    WRENCH_INFO("Loading workflow from: %s", workflow_file);
    wrench::Workflow *workflow;
    workflow = wrench::PegasusWorkflowParser::createWorkflowFromJSON(workflow_file, "1f");

    WRENCH_INFO("The workflow has %ld tasks", workflow->getNumberOfTasks());
    std::cerr << "Total Number of Workflow Tasks: " << workflow->getNumberOfTasks() << std::endl;

    std::string wms_host = "master";

    // file registry service
    WRENCH_INFO("Instantiating a FileRegistryService on: %s", wms_host.c_str());
    std::shared_ptr<wrench::FileRegistryService> file_registry_service = simulation.add(
            new wrench::FileRegistryService(wms_host));

    // compute services
    std::set<std::shared_ptr<wrench::ComputeService>> compute_services;
    std::vector<std::string> hosts{"worker1", "worker2", "worker3", "worker4"};
    auto cloud_service = simulation.add(new wrench::CloudComputeService(wms_host, hosts, {"/"}, {}, {
            {wrench::CloudComputeServiceMessagePayload::START_VM_REQUEST_MESSAGE_PAYLOAD,    1024},
            {wrench::CloudComputeServiceMessagePayload::SHUTDOWN_VM_REQUEST_MESSAGE_PAYLOAD, 1024},
    }));
    compute_services.insert(cloud_service);

    // storage services
    std::string storage_host = "data_server";
    std::shared_ptr<wrench::StorageService> storage_service = simulation.add(
            new wrench::SimpleStorageService(storage_host, {"/"}));

    // scheduling algorithm
//    auto scheduling_algorithm = std::make_unique<SPSSEBAlgorithm>(
//    auto scheduling_algorithm = std::make_unique<IOAwareAlgorithm>(
//    auto scheduling_algorithm = std::make_unique<IOAwareBalanceAlgorithm>(
    auto scheduling_algorithm = std::make_unique<EnRealAlgorithm>(
            cloud_service,
            std::make_unique<TraditionalPowerModel>(cloud_service));

    // instantiate the wms
    auto wms = simulation.add(
            new GreedyWMS(std::make_unique<EnergyAwareStandardJobScheduler>(
                    storage_service, std::move(scheduling_algorithm)),
                          compute_services, {storage_service}, wms_host));

    wms->addWorkflow(workflow);

    // stage input data
    WRENCH_INFO("Staging workflow input files to external Storage Service...");
    for (auto file : workflow->getInputFiles()) {
        simulation.stageFile(file, storage_service);
    }

    // simulation execution
    WRENCH_INFO("Launching the Simulation...");
    try {
        simulation.launch();
    } catch (std::runtime_error &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 0;
    }

    WRENCH_INFO("Simulation done!");

    // json output file
    simulation.getOutput().dumpUnifiedJSON(workflow, "tmp.json");

    // statistics
    auto power_trace = simulation.getOutput().getTrace<wrench::SimulationTimestampEnergyConsumption>();
    double previous_traditional_date = 0;
    double previous_pairwise_date = 0;
    double previous_unpaired_date = 0;
    std::map<std::string, double> workers_traditional_power;
    std::map<std::string, double> workers_pairwise_power;
    std::map<std::string, double> workers_unpaired_power;

    for (auto &host : hosts) {
        workers_traditional_power.insert(std::pair<std::string, double>(host, 0));
        workers_pairwise_power.insert(std::pair<std::string, double>(host, 0));
        workers_unpaired_power.insert(std::pair<std::string, double>(host, 0));
    }
    for (auto measurement : power_trace) {
        auto key = measurement->getContent()->getHostname();
        auto model = key.substr(0, key.find("__"));
        auto hostname = key.substr(key.find("__") + 2, key.size());

        if (model == "traditional") {
            auto diff = (measurement->getContent()->getDate() - previous_traditional_date);
            workers_traditional_power.at(hostname) += measurement->getContent()->getConsumption() *
                                                      ((diff > 0 ? diff : 1) / 3600.0);
            previous_traditional_date = measurement->getContent()->getDate();

        } else if (model == "pairwise") {
            auto diff = (measurement->getContent()->getDate() - previous_pairwise_date);
            workers_pairwise_power.at(hostname) += measurement->getContent()->getConsumption() *
                                                   ((diff > 0 ? diff : 1) / 3600.0);
            previous_pairwise_date = measurement->getContent()->getDate();

        } else if (model == "unpaired") {
            auto diff = (measurement->getContent()->getDate() - previous_unpaired_date);
            workers_unpaired_power.at(hostname) += measurement->getContent()->getConsumption() *
                                                   ((diff > 0 ? diff : 1) / 3600.0);
            previous_unpaired_date = measurement->getContent()->getDate();
        }
    }

    double total_traditional_energy = 0;
    double total_pairwise_energy = 0;
    double total_unpaired_energy = 0;

    for (auto &host : hosts) {
        total_traditional_energy += workers_traditional_power.at(host);
        total_pairwise_energy += workers_pairwise_power.at(host);
        total_unpaired_energy += workers_unpaired_power.at(host);
    }
    std::cerr << "Workflow Makespan (s): " << wrench::Simulation::getCurrentSimulatedDate() << std::endl;
    std::cerr << "Total Traditional Energy (Wh): " << total_traditional_energy << std::endl;
    std::cerr << "Total Pairwise Energy (Wh): " << total_pairwise_energy << std::endl;
    std::cerr << "Total Unpaired Energy (Wh): " << total_unpaired_energy << std::endl;
    std::cerr << std::endl;
    std::cerr << argv[3] << "," << workflow->getNumberOfTasks() << ",EnReal,traditional,"
              << total_traditional_energy << "," << wrench::Simulation::getCurrentSimulatedDate() << std::endl;
    std::cerr << argv[3] << "," << workflow->getNumberOfTasks() << ",EnReal,pairwise,"
              << total_pairwise_energy << "," << wrench::Simulation::getCurrentSimulatedDate() << std::endl;
    std::cerr << argv[3] << "," << workflow->getNumberOfTasks() << ",EnReal,unpaired,"
              << total_unpaired_energy << "," << wrench::Simulation::getCurrentSimulatedDate() << std::endl;
    return 0;
}
