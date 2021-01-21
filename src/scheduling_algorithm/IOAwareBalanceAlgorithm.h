/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ENERGY_AWARE_IOAWAREBALANCEALGORITHM_H
#define ENERGY_AWARE_IOAWAREBALANCEALGORITHM_H

#include "IOAwareAlgorithm.h"

class IOAwareBalanceAlgorithm : public IOAwareAlgorithm {
public:
    IOAwareBalanceAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service,
                            std::unique_ptr<CostModel> cost_model);

    std::vector<wrench::WorkflowTask *> sortTasks(const std::vector<wrench::WorkflowTask *> &tasks) override;
};


#endif //ENERGY_AWARE_IOAWAREBALANCEALGORITHM_H
