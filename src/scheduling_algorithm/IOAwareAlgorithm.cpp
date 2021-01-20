/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "IOAwareAlgorithm.h"

WRENCH_LOG_CATEGORY(ioaware_algorithm, "Log category for IOAwareAlgorithm");

/**
 *
 * @param cloud_service
 * @param power_model
 */
IOAwareAlgorithm::IOAwareAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service,
                                   std::unique_ptr<CostModel> cost_model)
        : SchedulingAlgorithm(cloud_service, std::move(cost_model)) {
}

/**
 *
 * @param tasks
 * @return
 */
std::vector<wrench::WorkflowTask *> IOAwareAlgorithm::sortTasks(const vector<wrench::WorkflowTask *> &tasks) {
    auto sorted_tasks = tasks;

    std::sort(sorted_tasks.begin(), sorted_tasks.end(),
              [](const wrench::WorkflowTask *t1, const wrench::WorkflowTask *t2) -> bool {
                  if (t1->getAverageCPU() == t2->getAverageCPU()) {
                      return ((uintptr_t) t1 < (uintptr_t) t2);
                  } else {
                      return (t1->getAverageCPU() > t2->getAverageCPU());
                  }
              });

    int task_begin_shift = 0;
    int task_end_shift = 0;

    // plan tasks depending on cpu usage
    this->task_to_host_schedule.clear();
    auto num_cores_host = this->cloud_service->getPerHostNumCores();
    auto idle_cores_host = this->cloud_service->getPerHostNumIdleCores();
    std::vector<std::string> scheduled_vms;
    bool begin = true;

//    while (task_end_shift < (sorted_tasks.size() + 1)/ 2) {
    for (auto task : sorted_tasks) {
        std::string candidate_host;
        unsigned long candidate_host_used_cores = LONG_MAX;

        // look for existing VMs
        for (auto &it : this->vm_worker_map) {
            if ((this->cloud_service->isVMRunning(it.first) &&
                 this->cloud_service->getVMComputeService(it.first)->getTotalNumIdleCores() > 0) ||
                this->cloud_service->isVMDown(it.first)) {
                if (!std::count(scheduled_vms.begin(), scheduled_vms.end(), it.first)) {
                    scheduled_vms.push_back(it.first);
                    candidate_host = it.second;
                    candidate_host_used_cores = this->cloud_service->getPerHostNumIdleCores().at(it.second);
                    break;
                }
            }
        }

        if (candidate_host.empty()) {
            unsigned long candidate_host_idle_cores = LONG_MAX;
            // find candidate host
            for (auto &it : idle_cores_host) {
                if (it.second > 0 && it.second < candidate_host_idle_cores) {
                    candidate_host = it.first;
                    candidate_host_idle_cores = it.second;
                }
            }
        }
        if (candidate_host.empty()) {
            break;
        }
        idle_cores_host[candidate_host]--;

//        auto task = begin ? *(sorted_tasks.begin() + task_begin_shift) : *(sorted_tasks.end() - task_end_shift - 1);
//        begin ? task_begin_shift++ : task_end_shift++;
//        begin = !begin;
//        std::cerr << "CANDIDATE HOST: " << candidate_host << " - " << task->getID() << std::endl;

        this->task_to_host_schedule.insert(std::pair<wrench::WorkflowTask *, std::string>(task, candidate_host));

//        // shifting tasks indexes
//        if (task_begin == task_end) {
//            std::cerr << "EQUALS" << std::endl;
//            break;
//        } else {
//            std::cerr << "PAIR: " << task_begin->getID() << " - " << task_end->getID() << std::endl;
//            task_begin_shift++;
//            task_end_shift++;
//        }

    }
    return sorted_tasks;
}

/**
 *
 * @param task
 * @return
 */
std::string IOAwareAlgorithm::scheduleTask(const wrench::WorkflowTask *task) {

    if (this->task_to_host_schedule.find(task) == this->task_to_host_schedule.end()) {
        return "";
    }

    auto host = this->task_to_host_schedule.at(task);
    if (!wrench::Simulation::isHostOn(host)) {
        wrench::Simulation::turnOnHost(host);
    }

    // find candidate vms
    std::vector<std::string> candidate_vms;
    for (auto &it : this->vm_worker_map) {
        if (it.second == host) {
            candidate_vms.push_back(it.first);
        }
    }

    // look for a running, idle VM
    std::string vm_name;
    for (const auto &vm : candidate_vms) {
        if (this->cloud_service->isVMRunning(vm) &&
            this->cloud_service->getVMComputeService(vm)->getTotalNumIdleCores() > 0) {
            return vm;

        } else if (vm_name.empty() && this->cloud_service->isVMDown(vm)) {
            vm_name = vm;
        }
    }

    if (vm_name.empty()) {
        // create VM, as no viable VM could be found
        if (this->cloud_service->getPerHostNumIdleCores().at(host) == 0) {
            return "";
        }
        vm_name = this->cloud_service->createVM(1, 1000000000);
    }

    // start VM
//    std::cerr << "START VM FOR: " << task->getID() << " - " << vm_name << " - " << host << std::endl;
    this->cloud_service->startVM(vm_name);
    auto vm_pm = this->cloud_service->getVMPhysicalHostname(vm_name);

    if (this->vm_worker_map.find(vm_name) == this->vm_worker_map.end()) {
        this->vm_worker_map.insert(std::pair<std::string, std::string>(vm_name, vm_pm));

        if (this->worker_running_vms.find(vm_pm) == this->worker_running_vms.end()) {
            this->worker_running_vms.insert(std::pair<std::string, int>(vm_pm, 0));
        }
    }
    this->worker_running_vms.at(vm_pm)++;

    return vm_name;
}

/**
 *
 * @param vm_name
 * @param vm_pm
 */
void IOAwareAlgorithm::notifyVMShutdown(const string &vm_name, const string &vm_pm) {
    this->worker_running_vms.at(vm_pm)--;
    if (this->worker_running_vms.at(vm_pm) == 0) {
        wrench::Simulation::turnOffHost(vm_pm);
    }
}
