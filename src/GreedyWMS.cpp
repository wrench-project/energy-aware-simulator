/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

/**
 ** A Workflow Management System (WMS) implementation that greedily
 ** assigns tasks to compute services.
 **/

#include <iostream>

#include "EnergyAwareStandardJobScheduler.h"
#include "GreedyWMS.h"

WRENCH_LOG_CATEGORY(greedy_wms, "Log category for GreedyWMS");

/**
 * @brief Constructor, which calls the super constructor
 *
 * @param compute_services: a set of compute services available to run tasks
 * @param storage_services: a set of storage services available to store files
 * @param hostname: the name of the host on which to start the WMS
 */
GreedyWMS::GreedyWMS(std::unique_ptr<wrench::StandardJobScheduler> standard_job_scheduler,
                     const std::set<std::shared_ptr<wrench::ComputeService>> &compute_services,
                     const std::set<std::shared_ptr<wrench::StorageService>> &storage_services,
                     const std::string &hostname) : WMS(std::move(standard_job_scheduler),
                                                        nullptr,
                                                        compute_services,
                                                        storage_services,
                                                        {}, nullptr,
                                                        hostname,
                                                        "greedy_wms") {}

/**
 * @brief main method of the GreedyWMS daemon
 *
 * @return 0 on completion
 *
 * @throw std::runtime_error
 */
int GreedyWMS::main() {
    /* Set the logging output to GREEN */
    wrench::TerminalOutput::setThisProcessLoggingColor(wrench::TerminalOutput::COLOR_GREEN);

    WRENCH_INFO("WMS starting on host %s", wrench::Simulation::getHostName().c_str());

    WRENCH_INFO("About to execute a workflow with %lu tasks", this->getWorkflow()->getNumberOfTasks());

    auto compute_services = this->getAvailableComputeServices<wrench::ComputeService>();

//    WRENCH_INFO("Initializing scheduler");
//    ((EnergyAwareStandardJobScheduler *) (this->getStandardJobScheduler()))->init(compute_services);

    // Create a job manager so that we can create/submit jobs
    auto job_manager = this->createJobManager();

    // While the workflow is not done, repeat the main loop
    while (not this->getWorkflow()->isDone()) {
        // Get the ready tasks
        auto ready_tasks = this->getWorkflow()->getReadyTasks();

        // Schedule them
        WRENCH_INFO("Scheduling tasks...");
        this->getStandardJobScheduler()->scheduleTasks(compute_services, ready_tasks);

        // Wait for a workflow execution event and process it
        WRENCH_INFO("Waiting for next event");
        this->waitForAndProcessNextEvent();
    }

    WRENCH_INFO("Workflow execution complete");
    return 0;
}

/**
 * @brief Process a standard job completion event
 *
 * @param event: the event
 */
void GreedyWMS::processEventStandardJobCompletion(std::shared_ptr<wrench::StandardJobCompletedEvent> event) {
    // Retrieve the job that this event is for
    auto job = event->standard_job;

    // Retrieve the job's tasks
    for (auto const &task : job->getTasks()) {
        // notify task completion
        WRENCH_INFO("Notified that a standard job has completed task %s", task->getID().c_str());
        auto scheduler = (EnergyAwareStandardJobScheduler *) (this->getStandardJobScheduler());
//         scheduler->updateCoreIdleness(cs, task->getExecutionHistory().top().num_cores_allocated);
        scheduler->notifyTaskCompletion(this->getAvailableComputeServices<wrench::ComputeService>(), task);
    }
}

/**
 * @brief Process a standard job failure event
 *
 * @param event: the event
 */
void GreedyWMS::processEventStandardJobFailure(std::shared_ptr<wrench::StandardJobFailedEvent> event) {
    // Retrieve the job that this event is for
    auto job = event->standard_job;

    WRENCH_INFO("Notified that a standard job has failed (failure cause: %s)",
                event->failure_cause->toString().c_str());

    WRENCH_INFO("As a result, the following tasks have failed:");
    for (auto const &task : job->getTasks()) {
        // Retrieve the job's tasks
        WRENCH_INFO(" - %s", task->getID().c_str());
    }
    throw std::runtime_error("This should not happen in this example");
}
