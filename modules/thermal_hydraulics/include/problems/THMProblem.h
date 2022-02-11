//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  virtual void copySolutionsBackwards() override;

  /**
   * Check if a postprocessor with a given name exists in the simulation
   *
   * @return true if postprocessor with the given name exists in the simulation, false otherwise
   * @param name The name of the postprocessor
   */
  virtual bool hasPostprocessor(const std::string & name) const;

public:
  static InputParameters validParams();
};
