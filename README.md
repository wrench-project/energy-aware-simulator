[![Build](https://github.com/wrench-project/energy-aware-simulator/workflows/Build/badge.svg)](https://github.com/wrench-project/energy-aware-simulator/actions)
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](LICENSE)
[![CodeFactor](https://www.codefactor.io/repository/github/wrench-project/energy-aware-simulator/badge)](https://www.codefactor.io/repository/github/wrench-project/energy-aware-simulator)

# Energy-Aware Simulator

An energy-aware workflow scheduling simulator developed with the 
[WRENCH](https://wrench-project.org) framework.

### Dependencies

- [SimGrid](http://simgrid.org) 3.26
- [WRENCH](https://wrench-project.org) 1.8-dev

### The Simulator

This simulator implements the following scheduling algorithms:

- Static Provisioning-Static Scheduling under Energy and Budget
  Constraints ([SPSS-EB](https://doi.org/10.1109/CGC.2013.14))
- Energy-aware Resource Allocation ([EnReal](https://doi.org/10.1109/TCC.2015.2453966))
