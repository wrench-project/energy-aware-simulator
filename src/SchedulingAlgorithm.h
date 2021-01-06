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

class SchedulingAlgorithm {

public:
    /**
     * @brief Constructor
     */
    explicit SchedulingAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service) :
            cloud_service(cloud_service) {}

    /***********************/
    /** \cond INTERNAL     */
    /***********************/
    virtual ~SchedulingAlgorithm() = default;
    /***********************/
    /** \endcond           */
    /***********************/

    virtual std::shared_ptr<wrench::BareMetalComputeService> scheduleTask(const wrench::WorkflowTask *task) = 0;

protected:
    std::shared_ptr<wrench::CloudComputeService> &cloud_service;
};

#endif //ENERGY_AWARE_SCHEDULINGALGORITHM_H
