/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "TraditionalPowerModel.h"

/**
 *
 * @param cloud_service
 */
TraditionalPowerModel::TraditionalPowerModel(shared_ptr<wrench::CloudComputeService> &cloud_service) :
        CostModel(cloud_service) {}

/**
 *
 * @param task
 * @param vm_name
 * @return
 */
double TraditionalPowerModel::estimateCost(const wrench::WorkflowTask *task, std::string vm_name) {
    return 0;
}
