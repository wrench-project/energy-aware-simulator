/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ENERGY_AWARE_SCHEDULINGALGORITHM_H
#define ENERGY_AWARE_SCHEDULINGALGORITHM_H

#include <wrench-dev.h>

#include "cost_model/CostModel.h"

class SchedulingAlgorithm {
public:
    /**
     * @brief Constructor
     */
    explicit SchedulingAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service,
                                 std::unique_ptr<CostModel> cost_model) :
            cloud_service(cloud_service), cost_model(std::move(cost_model)) {}

    virtual ~SchedulingAlgorithm() = default;

    virtual std::string scheduleTask(const wrench::WorkflowTask *task) = 0;

protected:
    std::shared_ptr<wrench::CloudComputeService> &cloud_service;
    std::unique_ptr<CostModel> cost_model;
};

#endif //ENERGY_AWARE_SCHEDULINGALGORITHM_H
