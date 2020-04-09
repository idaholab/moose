#pragma once

#include "FEProblem.h"
#include "Simulation.h"

/**
 * Specialization of FEProblem to run with component subsystem
 */
class THMProblem : public FEProblem, public Simulation
{
public:
  THMProblem(const InputParameters & parameters);

  virtual void advanceState() override;

public:
  static InputParameters validParams();
};
