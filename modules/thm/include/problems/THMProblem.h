#pragma once

#include "FEProblem.h"
#include "Simulation.h"

class THMProblem;

template <>
InputParameters validParams<THMProblem>();

/**
 * Specialization of FEProblem to run with component subsystem
 */
class THMProblem : public FEProblem, public Simulation
{
public:
  THMProblem(const InputParameters & parameters);
};
