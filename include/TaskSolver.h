#pragma once

#include "SimulationData.h"

class TaskSolver {
public:
    static SimulationResults solveTest(const TestParams& params);
    static SimulationResults solveMain(const MainParams& params);
};
