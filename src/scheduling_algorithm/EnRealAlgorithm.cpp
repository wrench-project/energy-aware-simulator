/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "EnRealAlgorithm.h"

WRENCH_LOG_CATEGORY(enreal_algorithm, "Log category for EnRealAlgorithm");

/**
 *
 * @param cloud_service
 * @param power_model
 */
EnRealAlgorithm::EnRealAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service,
                                 std::unique_ptr<CostModel> cost_model)
        : SPSSEBAlgorithm(cloud_service, std::move(cost_model)) {}

/**
 *
 * @param task
 * @return
 */
std::string EnRealAlgorithm::scheduleTask(const wrench::WorkflowTask *task) {

//    auto host = this->task_to_host_schedule.at(task);
//    if (!wrench::Simulation::isHostOn(host)) {
//        wrench::Simulation::turnOnHost(host);
//    }

    // find candidate vms
    std::vector<std::string> candidate_vms;
    for (auto &it : this->vm_worker_map) {
        candidate_vms.push_back(it.first);
    }

    // look for a running, idle VM
    std::string vm_name;
    for (const auto &vm : candidate_vms) {
        if (this->cloud_service->isVMRunning(vm) &&
            this->cloud_service->getVMComputeService(vm)->getTotalNumIdleCores() > 0) {
            return vm;

        } else if (vm_name.empty() && this->cloud_service->isVMDown(vm)) {
            vm_name = vm;
        }
    }

    if (vm_name.empty()) {
        // create VM, as no viable VM could be found
        if (this->cloud_service->getTotalNumIdleCores() == 0) {
            return "";
        }
        bool turned_on = false;
        for (auto &host : this->cloud_service->getExecutionHosts()) {
            if (!wrench::Simulation::isHostOn(host)) {
                wrench::Simulation::turnOnHost(host);
                turned_on = true;
                break;
            }
        }
        if (turned_on) {
            vm_name = this->cloud_service->createVM(1, 1000000000);
        }
    }

    if (vm_name.empty()) {
        return "";
    }

    // start VM
    this->cloud_service->startVM(vm_name);
    auto vm_pm = this->cloud_service->getVMPhysicalHostname(vm_name);

    if (this->vm_worker_map.find(vm_name) == this->vm_worker_map.end()) {
        this->vm_worker_map.insert(std::pair<std::string, std::string>(vm_name, vm_pm));

        if (this->worker_running_vms.find(vm_pm) == this->worker_running_vms.end()) {
            this->worker_running_vms.insert(std::pair<std::string, int>(vm_pm, 0));
        }
    }
    this->worker_running_vms.at(vm_pm)++;

    return vm_name;
}

void EnRealAlgorithm::notifyVMShutdown(const std::string &vm_name, const std::string &vm_pm) {
    this->vm_worker_map.erase(vm_name);
    this->worker_running_vms.at(vm_pm)--;
    if (this->worker_running_vms.at(vm_pm) == 0) {
        wrench::Simulation::turnOffHost(vm_pm);
    }
}
