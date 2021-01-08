/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "SPSSEBAlgorithm.h"

typedef double d;
WRENCH_LOG_CATEGORY(spss_eb_algorithm, "Log category for SPSSEBAlgorithm");

/**
 *
 * @param cloud_service
 * @param power_model
 */
SPSSEBAlgorithm::SPSSEBAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service,
                                 std::unique_ptr<CostModel> cost_model)
        : SchedulingAlgorithm(cloud_service, std::move(cost_model)) {}

/**
 *
 * @param task
 * @return
 */
std::string SPSSEBAlgorithm::scheduleTask(const wrench::WorkflowTask *task) {
    std::vector<std::string> candidate_vms;

    // look for existing VMs
    for (const auto &vm : this->vms_pool) {
        if (this->cloud_service->isVMRunning(vm) &&
            this->cloud_service->getVMComputeService(vm)->getTotalNumIdleCores() > 0) {
            candidate_vms.push_back(vm);
        } else if (this->cloud_service->isVMDown(vm)) {
            candidate_vms.push_back(vm);
        }
    }

    // get VM with minimum cost
    std::string vm_name;
    double min_cost = numeric_limits<double>::max();
    for (const auto &vm : candidate_vms) {
        double cost = this->cost_model->estimateCost(task, vm);
        if (cost < min_cost) {
            min_cost = cost;
            vm_name = vm;
        }
    }

    // if VM is down, start it
    if (!vm_name.empty() && this->cloud_service->isVMDown(vm_name)) {
        this->cloud_service->startVM(vm_name);
        return vm_name;
    }

    // if task cannot start now on a running VM, it will start a new VM if possible
    if (vm_name.empty() && this->cloud_service->getTotalNumIdleCores() > 0) {
        vm_name = this->cloud_service->createVM(1, 1000000000);
        this->cloud_service->startVM(vm_name);
        this->vms_pool.insert(vm_name);
    }

    return vm_name;
}
