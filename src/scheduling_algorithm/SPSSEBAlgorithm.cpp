/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "SPSSEBAlgorithm.h"

WRENCH_LOG_CATEGORY(spss_eb_algorithm, "Log category for SPSSEBAlgorithm");

/**
 *
 * @param cloud_service
 * @param power_model
 */
SPSSEBAlgorithm::SPSSEBAlgorithm(std::shared_ptr<wrench::CloudComputeService> &cloud_service,
                                 std::unique_ptr<CostModel> cost_model)
        : SchedulingAlgorithm(cloud_service, std::move(cost_model)) {
}

/**
 *
 * @param tasks
 * @return
 */
std::vector<wrench::WorkflowTask *> SPSSEBAlgorithm::sortTasks(const std::vector<wrench::WorkflowTask *> &tasks) {
    auto sorted_tasks = tasks;

    std::sort(sorted_tasks.begin(), sorted_tasks.end(),
              [](const wrench::WorkflowTask *t1, const wrench::WorkflowTask *t2) -> bool {
                  if (t1->getFlops() == t2->getFlops()) {
                      return ((uintptr_t) t1 < (uintptr_t) t2);
                  } else {
                      return (t1->getFlops() > t2->getFlops());
                  }
              });

    return sorted_tasks;
}

/**
 *
 * @param task
 * @return
 */
std::string SPSSEBAlgorithm::scheduleTask(const wrench::WorkflowTask *task) {
    std::vector<std::string> candidate_vms;

    // look for existing VMs
    for (const auto &vm : this->vms_pool) {
        if (this->cloud_service->isVMRunning(vm) &&
            this->cloud_service->getVMComputeService(vm)->getTotalNumIdleCores() > 0) {
            candidate_vms.push_back(vm);
        } else if (this->cloud_service->isVMDown(vm)) {
            candidate_vms.push_back(vm);
        }
    }

    // get VM with minimum cost
    std::string vm_name;
    double min_cost = numeric_limits<double>::max();
    for (const auto &vm : candidate_vms) {
        double cost = this->cost_model->estimateCost(task, vm, this->worker_running_vms);
        if (cost < min_cost) {
            min_cost = cost;
            vm_name = vm;
        }
    }

    // if there is no cores available on running hosts, turn on another host
    bool has_idle_host = false;
    for (auto &it : this->worker_running_vms) {
        if (wrench::Simulation::isHostOn(it.first) && it.second < wrench::Simulation::getHostNumCores(it.first)) {
            has_idle_host = true;
            break;
        }
    }
    if (!has_idle_host) {
        for (auto &host : this->cloud_service->getExecutionHosts()) {
            if (!wrench::Simulation::isHostOn(host)) {
                std::cerr << "======= TURNING ON: " << host << std::endl;
                wrench::Simulation::turnOnHost(host);
                break;
            }
        }
    }

    // if VM is down, start it
    if (!vm_name.empty() && this->cloud_service->isVMDown(vm_name)) {
        this->cloud_service->startVM(vm_name);
        auto vm_pm = this->cloud_service->getVMPhysicalHostname(vm_name);

        if (this->vm_worker_map.find(vm_name) == this->vm_worker_map.end()) {
            this->vm_worker_map.insert(std::pair<std::string, std::string>(vm_name, vm_pm));
        } else {
            this->vm_worker_map.at(vm_name) = vm_pm;
        }

        if (this->worker_running_vms.find(vm_pm) == this->worker_running_vms.end()) {
            this->worker_running_vms.insert(std::pair<std::string, int>(vm_pm, 1));
        } else {
            this->worker_running_vms.at(vm_pm)++;
        };

//        if (!wrench::Simulation::isHostOn(vm_pm)) {
//            std::cerr << "======= TURNING ON: " << vm_pm << std::endl;
//            wrench::Simulation::turnOnHost(vm_pm);
//        }
//        if (this->worker_running_vms.at(vm_pm) == 4) {
//            std::cerr << "======== 1 WORKER is 4: " << vm_pm << std::endl;
//            for (auto &it : this->worker_running_vms) {
//                std::cerr << "========== 1 [" << it.first << ", " << it.second << "]" << std::endl;
//            }
//        }
        return vm_name;
    }

    // if task cannot start now on a running VM, it will start a new VM if possible
    if (vm_name.empty() && this->cloud_service->getTotalNumIdleCores() > 0) {

        vm_name = this->cloud_service->createVM(1, 1000000000);
        this->cloud_service->startVM(vm_name);
        this->vms_pool.insert(vm_name);

        auto vm_pm = this->cloud_service->getVMPhysicalHostname(vm_name);
        if (this->worker_running_vms.find(vm_pm) == this->worker_running_vms.end()) {
            this->worker_running_vms.insert(std::pair<std::string, int>(vm_pm, 0));
        }

//        std::cerr << "======== CREATED AND STARTED: " << vm_name << " - " << vm_pm << std::endl;
//
//        if (this->worker_running_vms.at(vm_pm) == 4) {
//            std::cerr << "======== 2 WORKER is 4: " << vm_pm << " - " << vm_name << std::endl;
//            for (auto &it : this->worker_running_vms) {
//                std::cerr << "========== 2 [" << it.first << ", " << it.second << "]" << std::endl;
//            }
//        }
        this->worker_running_vms.at(vm_pm)++;
//        for (auto &it : this->worker_running_vms) {
//            std::cerr << "========== 2 [" << it.first << ", " << it.second << "]" << std::endl;
//        }
        this->vm_worker_map.insert(std::pair<std::string, std::string>(vm_name, vm_pm));
    }

    return vm_name;
}

void SPSSEBAlgorithm::notifyVMShutdown(const std::string &vm_name, const std::string &vm_pm) {
    this->worker_running_vms.at(vm_pm)--;
//    for (auto &it : this->worker_running_vms) {
//        std::cerr << "========== [" << it.first << ", " << it.second << "]" << std::endl;
//    }
    if (this->worker_running_vms.at(vm_pm) == 0) {
        std::cerr << "======= TURNING OFF: " << vm_pm << std::endl;
        wrench::Simulation::turnOffHost(vm_pm);
    }
}
