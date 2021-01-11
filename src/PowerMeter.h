/**
 * Copyright (c) 2020-2021. The WRENCH Team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef ENERGY_AWARE_POWERMETER_H
#define ENERGY_AWARE_POWERMETER_H

#include <wrench-dev.h>

class PowerMeter : public wrench::Service {
public:
    PowerMeter(wrench::WMS *wms,
               const std::vector<std::string> &hostnames,
               double period,
               bool traditional = true,
               bool pairwise = false);

    void stop() override;

private:
    int main() override;

    struct TaskStartTimeComparator {
        bool operator()(wrench::WorkflowTask *&lhs, wrench::WorkflowTask *&rhs);
    };

    void computePowerMeasurements(const std::string &hostname, std::set<wrench::WorkflowTask *> &tasks);

    bool processNextMessage(double timeout);

    wrench::WMS *wms;
    bool traditional;
    bool pairwise;
    double measurement_period;
    double time_to_next_measurement;
};

#endif //ENERGY_AWARE_POWERMETER_H
