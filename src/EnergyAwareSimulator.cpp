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
#include "scheduling_algorithm/SPSSEBAlgorithm.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(EnergyAwareSimulator, "Log category for EnergyAwareSimulator");

int main(int argc, char **argv) {
    // create and initialize the simulation
    wrench::Simulation simulation;
    simulation.init(&argc, argv);

    // check to make sure there are the right number of arguments
    if (argc != 3) {
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

    std::string wms_host = "master";

    // file registry service
    WRENCH_INFO("Instantiating a FileRegistryService on: %s", wms_host.c_str());
    std::shared_ptr<wrench::FileRegistryService> file_registry_service = simulation.add(
            new wrench::FileRegistryService(wms_host));

    // compute services
    std::set<std::shared_ptr<wrench::ComputeService>> compute_services;
    std::vector<std::string> hosts{"worker1", "worker2"};
    auto cloud_service = simulation.add(new wrench::CloudComputeService(wms_host, hosts, {"/"}, {}, {
            {wrench::CloudComputeServiceMessagePayload::START_VM_REQUEST_MESSAGE_PAYLOAD,    1024},
            {wrench::CloudComputeServiceMessagePayload::SHUTDOWN_VM_REQUEST_MESSAGE_PAYLOAD, 1024},
    }));
    compute_services.insert(cloud_service);

    // storage services
    std::string storage_host = "data_server";
    std::shared_ptr<wrench::StorageService> storage_service = simulation.add(
            new wrench::SimpleStorageService(storage_host, {"/"}));

    // instantiate the wms
    auto wms = simulation.add(
            new GreedyWMS(std::make_unique<EnergyAwareStandardJobScheduler>(
                    storage_service,
                    std::make_unique<SPSSEBAlgorithm>(
                            cloud_service,
                            std::make_unique<TraditionalPowerModel>(cloud_service))),
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

    // statistics
    simulation.getOutput().dumpUnifiedJSON(workflow, "tmp.json");

    return 0;
}
