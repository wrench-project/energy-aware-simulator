/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ENERGY_AWARE_TRADITIONALPOWERMODEL_H
#define ENERGY_AWARE_TRADITIONALPOWERMODEL_H

#include "CostModel.h"

class TraditionalPowerModel : public CostModel {
public:
    explicit TraditionalPowerModel(std::shared_ptr<wrench::CloudComputeService> &cloud_service);

    double estimateCost(const wrench::WorkflowTask *task, std::string vm_name) override;
};

#endif //ENERGY_AWARE_TRADITIONALPOWERMODEL_H
