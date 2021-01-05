/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ENERGY_AWARE_GREEDYWMS_H
#define ENERGY_AWARE_GREEDYWMS_H

#include <wrench-dev.h>

/**
 *  @brief A Workflow Management System (WMS) implementation that greedily
 *         assigns tasks to compute services.
 */
class GreedyWMS : public wrench::WMS {

public:
    // Constructor
    GreedyWMS(std::unique_ptr<wrench::StandardJobScheduler> standard_job_scheduler,
              const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
              const std::set<std::shared_ptr<wrench::StorageService>> &storage_services,
              const std::string &hostname);

protected:
    // Overridden method
    void processEventStandardJobCompletion(std::shared_ptr<wrench::StandardJobCompletedEvent>) override;

    void processEventStandardJobFailure(std::shared_ptr<wrench::StandardJobFailedEvent>) override;

private:
    // main() method of the WMS
    int main() override;
};

#endif //ENERGY_AWARE_GREEDYWMS_H
