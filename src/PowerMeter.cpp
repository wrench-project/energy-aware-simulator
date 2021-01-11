/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "PowerMeter.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(power_meter, "Log category for PowerMeter");

/**
 * @brief Constructor
 *
 * @param wms: the WMS that uses this power meter
 * @param hostnames: the list of metered hosts, as hostnames
 * @param measurement_period: the measurement period
 * @param traditional: whether the traditional power model should be used
 * @param pairwise: whether cores in socket are enabled in pairwise manner
 */
PowerMeter::PowerMeter(wrench::WMS *wms,
                       const std::vector<std::string> &hostnames,
                       double measurement_period,
                       bool traditional,
                       bool pairwise) :
        Service(wms->hostname, "power_meter", "power_meter"),
        wms(wms),
        traditional(traditional),
        pairwise(pairwise),
        measurement_period(measurement_period) {
    // sanity checks
    if (hostnames.empty()) {
        throw std::invalid_argument("PowerMeter::PowerMeter(): no host to meter!");
    }
    if (measurement_period < 1) {
        throw std::invalid_argument("PowerMeter::PowerMeter(): measurement period must be at least 1 second");
    }
    for (auto const &h : hostnames) {
        if (not wrench::S4U_Simulation::hostExists(h)) {
            throw std::invalid_argument("PowerMeter::PowerMeter(): unknown host " + h);
        }
    }

    this->time_to_next_measurement = 0.0;
}

/**
 * @brief Compare the start time between two workflow tasks
 *
 * @param lhs: pointer to a workflow task
 * @param rhs: pointer to a workflow task
 *
 * @return whether the start time of the left-hand-side workflow tasks is earlier
 */
bool PowerMeter::TaskStartTimeComparator::operator()(wrench::WorkflowTask *&lhs, wrench::WorkflowTask *&rhs) {
    return lhs->getStartDate() < rhs->getStartDate();
}

/**
 * @brief Main method of the daemon that implements the PowerMeter
 * @return 0 on success
 */
int PowerMeter::main() {
    wrench::TerminalOutput::setThisProcessLoggingColor(wrench::TerminalOutput::COLOR_YELLOW);

    WRENCH_INFO("New Power Meter starting (%s)", this->mailbox_name.c_str());

    /** Main loop **/
    while (true) {
        wrench::S4U_Simulation::computeZeroFlop();

        double current_time = wrench::Simulation::getCurrentSimulatedDate();

        if (current_time >= this->time_to_next_measurement) {
            // separate tasks per host
            std::map<std::string, std::set<wrench::WorkflowTask *>> tasks_per_host;

            for (auto task : this->wms->getWorkflow()->getTasks()) {

                // only process running tasks
                if (task->getStartDate() != -1 && task->getEndDate() == -1) {
                    if (tasks_per_host.find(task->getExecutionHost()) == tasks_per_host.end()) {
                        std::set<wrench::WorkflowTask *> tasks_set;
                        tasks_set.insert(task);
                        tasks_per_host.insert(std::make_pair(task->getExecutionHost(), tasks_set));
                    } else {
                        tasks_per_host[task->getExecutionHost()].insert(task);
                    }
                }
            }

            // compute power consumption
            for (auto &key_value : tasks_per_host) {
                this->computePowerMeasurements(key_value.first, key_value.second);
            }

            // update time to next measurement
            this->time_to_next_measurement = current_time + this->measurement_period;
        }

        // stop meter
        if (!this->processNextMessage(this->measurement_period)) {
            break;
        }
    }

    WRENCH_INFO("Energy Meter Manager terminating");
    return 0;
}

/**
 * @brief Obtain the current power consumption of a host and will add SimulationTimestampEnergyConsumption to
 *          simulation output if can_record is set to true
 *
 * @param hostname: the host name
 * @param tasks: list of WorkflowTask running on the host
 * @param record_as_time_stamp: bool signaling whether or not to record a SimulationTimestampEnergyConsumption object
 *
 * @throw std::invalid_argument
 */
void PowerMeter::computePowerMeasurements(const std::string &hostname,
                                          std::set<wrench::WorkflowTask *> &tasks) {
    int task_index = 0;
    double task_factor = 1;
    double consumption = wrench::S4U_Simulation::getMinPowerConsumption(hostname);

    for (auto task : tasks) {
        double task_consumption;

        if (this->traditional) {
            task_consumption = (wrench::Simulation::getMaxPowerConsumption(hostname) -
                                wrench::Simulation::getMinPowerConsumption(hostname)) /
                               double(wrench::Simulation::getHostNumCores(hostname));

        } else {
            // power related to cpu usage
            // dynamic power per socket
            double dynamic_power =
                    (wrench::Simulation::getMaxPowerConsumption(hostname) -
                     wrench::Simulation::getMinPowerConsumption(hostname)) *
                    (task->getAverageCPU() / 100) / 2;

            if (this->pairwise && task_index < 2) {
                task_consumption = dynamic_power / 6;

            } else if (this->pairwise && task_index >= 2) {
                task_consumption = task_factor * (dynamic_power / 6);
                task_factor *= 0.88;

            } else if (not this->pairwise && std::fmod(task_index, 6) == 0) {
                task_consumption = dynamic_power / 6;
                task_factor = 1;

            } else {
                task_consumption = task_factor * (dynamic_power / 6);
                task_factor *= 0.9;
            }

            // power related to IO usage
            task_consumption += task_consumption * (this->pairwise ? 0.486 : 0.213);

            // IOWait factor
            task_consumption *= 1.31;
            task_index++;
        }

        consumption += task_consumption;
    }

    this->simulation->getOutput().addTimestampEnergyConsumption(hostname, consumption);
}

/**
 * @brief Process the next message
 * @return true if the daemon should continue, false otherwise
 *
 * @throw std::runtime_error
 */
bool PowerMeter::processNextMessage(double timeout) {
    std::shared_ptr<wrench::SimulationMessage> message = nullptr;

    try {
        message = wrench::S4U_Mailbox::getMessage(this->mailbox_name, timeout);
    } catch (std::shared_ptr<wrench::NetworkError> &cause) {
        return true;
    }

    if (message == nullptr) { WRENCH_INFO("Got a NULL message... Likely this means we're all done. Aborting!");
        return false;
    }

    WRENCH_INFO("Power Meter got a %s message", message->getName().c_str());

    if (auto msg = dynamic_cast<wrench::ServiceStopDaemonMessage *>(message.get())) {
        // There shouldn't be any need to clean any state up
        return false;

    } else {
        throw std::runtime_error(
                "PowerMeter::waitForNextMessage(): Unexpected [" + message->getName() + "] message");
    }
}

/**
 * @brief Stop the power meter
 *
 * @throw WorkflowExecutionException
 * @throw std::runtime_error
 */
void PowerMeter::stop() {
    try {
        wrench::S4U_Mailbox::putMessage(this->mailbox_name, new wrench::ServiceStopDaemonMessage("", 0.0));
    } catch (std::shared_ptr<wrench::NetworkError> &cause) {
        throw wrench::WorkflowExecutionException(cause);
    }
}
