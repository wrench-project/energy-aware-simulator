/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "EnergyAwareStandardJobScheduler.h"

#include <utility>

WRENCH_LOG_CATEGORY(energy_aware_scheduler, "Log category for Energy Aware Scheduler");

/**
 * @brief Constructor, which calls the super constructor
 *
 * @param storage_service: default storage service available for the scheduler
 */
EnergyAwareStandardJobScheduler::EnergyAwareStandardJobScheduler(
        std::shared_ptr<wrench::StorageService> storage_service,
        std::unique_ptr<SchedulingAlgorithm> scheduling_algorithm) :
        default_storage_service(std::move(storage_service)), scheduling_algorithm(std::move(scheduling_algorithm)) {}

/**
 * @brief A method that schedules tasks (as part of standard jobs), according to whatever decision algorithm
 *        it implements, over a set of compute services
 * @param compute_services: the set of compute services
 * @param tasks: the set of tasks to be executed
 */
void EnergyAwareStandardJobScheduler::scheduleTasks(
        const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
        const std::vector<wrench::WorkflowTask *> &tasks) {

    // If nothing to do, return;
    if (compute_services.empty() or tasks.empty()) {
        return;
    } else if (compute_services.size() > 1) {
        throw std::runtime_error("This Energy-Aware Cloud Scheduler can only handle a single compute service");
    }

    WRENCH_INFO("There are %ld ready tasks to schedule", tasks.size());

    // Sort tasks by flops
    auto sorted_tasks = tasks;
    std::sort(sorted_tasks.begin(), sorted_tasks.end(),
              [](const wrench::WorkflowTask *t1, const wrench::WorkflowTask *t2) -> bool {
                  if (t1->getFlops() == t2->getFlops()) {
                      return ((uintptr_t) t1 < (uintptr_t) t2);
                  } else {
                      return (t1->getFlops() > t2->getFlops());
                  }
              });

    // obtaining cloud service
    auto cloud_service = std::dynamic_pointer_cast<wrench::CloudComputeService>(*compute_services.begin());
    if (cloud_service == nullptr) {
        throw std::runtime_error("This Energy-Aware Cloud Scheduler can only handle a cloud service");
    }

    // attempting to schedule tasks
    for (auto const &task : sorted_tasks) {
        auto vm_cs = this->scheduling_algorithm->scheduleTask(task);
//
//        if (cloud_service->getTotalNumIdleCores() > 0) {
//            std::string vm_name = cloud_service->createVM(1, 1000000000);
//            vm_cs = cloud_service->startVM(vm_name);
//            this->vms_pool.insert(vm_name);
//        } else {
//            for (const auto &vm_name : this->vms_pool) {
//                if (cloud_service->isVMRunning(vm_name)) {
//                    auto candidate_vm_cs = cloud_service->getVMComputeService(vm_name);
//                    if (candidate_vm_cs->getTotalNumIdleCores() > 0) {
//                        vm_cs = candidate_vm_cs;
//                        break;
//                    }
//                }
//            }
//        }
//

        if (vm_cs) {
            // finding the file locations
            std::map<wrench::WorkflowFile *, std::shared_ptr<wrench::FileLocation>> file_locations;
            for (auto f : task->getInputFiles()) {
                file_locations[f] = wrench::FileLocation::LOCATION(this->default_storage_service);
            }
            for (auto f : task->getOutputFiles()) {
                file_locations[f] = wrench::FileLocation::LOCATION(this->default_storage_service);
            }

            // creating job for execution
            std::shared_ptr<wrench::WorkflowJob> job =
                    (std::shared_ptr<wrench::WorkflowJob>) this->getJobManager()->createStandardJob(task,
                                                                                                    file_locations);

            WRENCH_INFO("Scheduling task: %s", task->getID().c_str());
            this->getJobManager()->submitJob(job, vm_cs);
        }
    }
}
