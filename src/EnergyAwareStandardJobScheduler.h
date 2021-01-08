/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ENERGY_AWARE_ENERGYAWARESTANDARDJOBSCHEDULER_H
#define ENERGY_AWARE_ENERGYAWARESTANDARDJOBSCHEDULER_H

#include <wrench-dev.h>

#include "cost_model/CostModel.h"
#include "scheduling_algorithm/SchedulingAlgorithm.h"

class EnergyAwareStandardJobScheduler : public wrench::StandardJobScheduler {

public:
    EnergyAwareStandardJobScheduler(std::shared_ptr<wrench::StorageService> storage_service,
                                    std::unique_ptr<SchedulingAlgorithm> scheduling_algorithm);

    void scheduleTasks(const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
                       const std::vector<wrench::WorkflowTask *> &tasks) override;

    void notifyTaskCompletion(const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
                              wrench::WorkflowTask *task);

private:
    std::shared_ptr<wrench::StorageService> default_storage_service;
    std::unique_ptr<SchedulingAlgorithm> scheduling_algorithm;
    int unscheduled_tasks;
    std::map<wrench::WorkflowTask *, std::string> tasks_vm_map;
};

#endif //ENERGY_AWARE_ENERGYAWARESTANDARDJOBSCHEDULER_H
