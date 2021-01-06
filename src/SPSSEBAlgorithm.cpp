/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "SPSSEBAlgorithm.h"

WRENCH_LOG_CATEGORY(spss_eb_algorithm, "Log category for SPSS EB Algorithm");

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
std::shared_ptr<wrench::BareMetalComputeService> SPSSEBAlgorithm::scheduleTask(const wrench::WorkflowTask *task) {
    std::shared_ptr<wrench::BareMetalComputeService> vm_cs = nullptr;

    if (cloud_service->getTotalNumIdleCores() > 0) {
        std::string vm_name = cloud_service->createVM(1, 1000000000);
        vm_cs = cloud_service->startVM(vm_name);
        this->vms_pool.insert(vm_name);
    } else {
        for (const auto &vm_name : this->vms_pool) {
            if (cloud_service->isVMRunning(vm_name)) {
                auto candidate_vm_cs = cloud_service->getVMComputeService(vm_name);
                if (candidate_vm_cs->getTotalNumIdleCores() > 0) {
                    vm_cs = candidate_vm_cs;
                    break;
                }
            }
        }
    }

    return vm_cs;
}
