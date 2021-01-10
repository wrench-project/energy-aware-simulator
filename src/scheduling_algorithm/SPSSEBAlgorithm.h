/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ENERGY_AWARE_SPSSEBALGORITHM_H
#define ENERGY_AWARE_SPSSEBALGORITHM_H

#include "SchedulingAlgorithm.h"
#include "cost_model/CostModel.h"

class SPSSEBAlgorithm : public SchedulingAlgorithm {
public:
    SPSSEBAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service,
                    std::unique_ptr<CostModel> cost_model);

    std::vector<wrench::WorkflowTask *> sortTasks(const std::vector<wrench::WorkflowTask *> &tasks) override;

    std::string scheduleTask(const wrench::WorkflowTask *task) override;

    void notifyVMShutdown(const std::string &vm_name, const std::string &vm_pm) override;

private:
    std::set<std::string> vms_pool;
    std::map<std::string, int> worker_running_vms;
};

#endif //ENERGY_AWARE_SPSSEBALGORITHM_H
