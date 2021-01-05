/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "EnergyAwareStandardJobScheduler.h"

/**
 * @brief Constructor, which calls the super constructor
 */
EnergyAwareStandardJobScheduler::EnergyAwareStandardJobScheduler() = default;

/**
 * @brief A method that schedules tasks (as part of standard jobs), according to whatever decision algorithm
 *        it implements, over a set of compute services
 * @param compute_services: the set of compute services
 * @param tasks: the set of tasks to be executed
 */
void EnergyAwareStandardJobScheduler::scheduleTasks(
        const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
        const std::vector<wrench::WorkflowTask *> &tasks) {

}
