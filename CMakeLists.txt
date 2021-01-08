cmake_minimum_required(VERSION 3.2)
message(STATUS "Cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")

project(energy-aware CXX)

add_definitions("-Wall -Wno-unused-variable -Wno-unused-private-field")

set(CMAKE_CXX_STANDARD 14)

# build the version number
set(ENERGY_AWARE_VERSION_MAJOR "1")
set(ENERGY_AWARE_VERSION_MINOR "0")
set(ENERGY_AWARE_VERSION_PATCH "0")
set(ENERGY_AWARE_VERSION_EXTRA "dev")

if (${ENERGY_AWARE_VERSION_PATCH} EQUAL "0")
    set(ENERGY_AWARE_RELEASE_VERSION "${ENERGY_AWARE_VERSION_MAJOR}.${ENERGY_AWARE_VERSION_MINOR}")
else ()
    set(ENERGY_AWARE_RELEASE_VERSION "$(ENERGY_AWARE_VERSION_MAJOR}.${ENERGY_AWARE_VERSION_MINOR}.${ENERGY_AWARE_VERSION_PATCH}")
endif ()

if (NOT ${ENERGY_AWARE_VERSION_EXTRA} EQUAL "")
    set(ENERGY_AWARE_RELEASE_VERSION "${ENERGY_AWARE_RELEASE_VERSION}-${ENERGY_AWARE_VERSION_EXTRA}")
endif ()

include_directories(src/ include/ /usr/local/include /usr/local/include/wrench)

# source files
set(SOURCE_FILES
        src/EnergyAwareSimulator.cpp
        src/EnergyAwareStandardJobScheduler.h
        src/EnergyAwareStandardJobScheduler.cpp
        src/GreedyWMS.h
        src/GreedyWMS.cpp
        src/cost_model/CostModel.h
        src/cost_model/TraditionalPowerModel.h
        src/cost_model/TraditionalPowerModel.cpp
        src/scheduling_algorithm/SchedulingAlgorithm.h
        src/scheduling_algorithm/SPSSEBAlgorithm.h
        src/scheduling_algorithm/SPSSEBAlgorithm.cpp
        )

set(TEST_FILES
        )

# wrench libraries
find_library(WRENCH_LIBRARY NAMES wrench)
find_library(WRENCH_PEGASUS_LIBRARY NAMES wrenchpegasusworkflowparser)
find_library(SIMGRID_LIBRARY NAMES simgrid)
find_library(PUGIXML_LIBRARY NAMES pugixml)
find_library(GTEST_LIBRARY NAMES gtest)

add_executable(wrench-energy-aware ${SOURCE_FILES})
target_link_libraries(wrench-energy-aware ${WRENCH_LIBRARY} ${WRENCH_PEGASUS_LIBRARY} ${SIMGRID_LIBRARY} ${PUGIXML_LIBRARY})
install(TARGETS wrench-energy-aware DESTINATION bin)
