/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "IOAwareBalanceAlgorithm.h"

WRENCH_LOG_CATEGORY(ioaware_balance_algorithm, "Log category for IOAwareBalanceAlgorithm");

/**
 *
 * @param cloud_service
 * @param power_model
 */
IOAwareBalanceAlgorithm::IOAwareBalanceAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service,
                                                 std::unique_ptr<CostModel> cost_model)
        : IOAwareAlgorithm(cloud_service, std::move(cost_model)) {
}

/**
 *
 * @param tasks
 * @return
 */
std::vector<wrench::WorkflowTask *> IOAwareBalanceAlgorithm::sortTasks(const vector<wrench::WorkflowTask *> &tasks) {
    auto sorted_tasks = tasks;

    std::sort(sorted_tasks.begin(), sorted_tasks.end(),
              [](const wrench::WorkflowTask *t1, const wrench::WorkflowTask *t2) -> bool {
                  if (t1->getAverageCPU() == t2->getAverageCPU()) {
                      return ((uintptr_t) t1 < (uintptr_t) t2);
                  } else {
                      return (t1->getAverageCPU() > t2->getAverageCPU());
                  }
              });

    // plan tasks depending on cpu usage
    auto num_cores_host = this->cloud_service->getPerHostNumCores();
    auto idle_cores_host = this->cloud_service->getPerHostNumIdleCores();
    std::vector<std::string> scheduled_vms;

    // balance tasks among hosts
    unsigned long max_num_full_hosts = std::floor((sorted_tasks.size() > 48 ? 48 : sorted_tasks.size()) / 12);
    unsigned long max_tasks_in_unfilled_host = (sorted_tasks.size() > 48 ? 48 : sorted_tasks.size()) % 12;
    auto hosts_list = this->cloud_service->getExecutionHosts();
    vector<wrench::WorkflowTask *> new_sort(sorted_tasks.size() > 48 ? 48 : sorted_tasks.size());

    int host_index = 0;
    int core_index = 0;

    for (std::size_t i = 0; i != new_sort.size(); ++i) {
        auto task = sorted_tasks[i];

        if ((host_index == max_num_full_hosts && core_index >= max_tasks_in_unfilled_host) ||
            host_index > max_num_full_hosts) {
            host_index = 0;
        }

        int idx = core_index + host_index * 12;
        new_sort[idx] = task;
        this->task_to_host_schedule.insert(
                std::pair<wrench::WorkflowTask *, std::string>(task, hosts_list[host_index]));

        host_index++;
        if (host_index == hosts_list.size()) {
            host_index = 0;
            core_index++;
        }
    }
    return new_sort;
}
