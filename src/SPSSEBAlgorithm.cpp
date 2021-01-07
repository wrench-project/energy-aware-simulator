/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "SPSSEBAlgorithm.h"

WRENCH_LOG_CATEGORY(spss_eb_algorithm, "Log category for SPSSEBAlgorithm");

/**
 *
 * @param cloud_service
 */
SPSSEBAlgorithm::SPSSEBAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service)
        : SchedulingAlgorithm(cloud_service) {}

/**
 *
 * @param task
 * @return
 */
std::string SPSSEBAlgorithm::scheduleTask(const wrench::WorkflowTask *task) {
    std::string vm_name;
    std::string vm_down_name;

    // look for existing VMs
    for (const auto &vm : this->vms_pool) {
        if (this->cloud_service->isVMRunning(vm)) {
            if (this->cloud_service->getVMComputeService(vm)->getTotalNumIdleCores() > 0) {
                vm_name = vm;
                break;
            }
        } else if (this->cloud_service->isVMDown(vm)) {
            vm_down_name = vm;
        }
    }

    if (vm_name.empty() && !vm_down_name.empty()) {
        this->cloud_service->startVM(vm_down_name);
        vm_name = vm_down_name;
    }

    // if task cannot start now on a running vm, it will start a new one if possible
    if (vm_name.empty() && this->cloud_service->getTotalNumIdleCores() > 0) {
        vm_name = this->cloud_service->createVM(1, 1000000000);
        this->cloud_service->startVM(vm_name);
        this->vms_pool.insert(vm_name);
    }

    return vm_name;
}
