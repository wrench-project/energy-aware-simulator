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

class EnergyAwareStandardJobScheduler : public wrench::StandardJobScheduler {

public:
    // Constructor
    EnergyAwareStandardJobScheduler();

    void scheduleTasks(const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
                       const std::vector<wrench::WorkflowTask *> &tasks) override;

};


#endif //ENERGY_AWARE_ENERGYAWARESTANDARDJOBSCHEDULER_H
