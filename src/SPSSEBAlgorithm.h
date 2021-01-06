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

class SPSSEBAlgorithm : public SchedulingAlgorithm {

public:
    explicit SPSSEBAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service);

    std::shared_ptr<wrench::BareMetalComputeService> scheduleTask(const wrench::WorkflowTask *task) override;

private:
    std::set<std::string> vms_pool;
};

#endif //ENERGY_AWARE_SPSSEBALGORITHM_H
