//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DISCRETENUCLEATIONTIMESTEP_H
#define DISCRETENUCLEATIONTIMESTEP_H

#include "GeneralPostprocessor.h"

class DiscreteNucleationTimeStep;
class DiscreteNucleationInserter;

template <>
InputParameters validParams<DiscreteNucleationTimeStep>();

/**
 * Returns a user defined timestep limit for the simulation step right after the
 * introduction of a new nucleus.
 */
class DiscreteNucleationTimeStep : public GeneralPostprocessor
{
public:
  DiscreteNucleationTimeStep(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual PostprocessorValue getValue() override;

protected:
  /// UserObject that manages nucleus insertion and deletion
  const DiscreteNucleationInserter & _inserter;

  /// User specified nucleation time step
  const Real _dt_nucleation;

  /**
   * Maximum total event expectation value that is low enough so that the
   * probability for more than one * nucleation event to occurr in a single
   * timestep is below a user specified value
   */
  Real _max_lambda;
};

#endif // DISCRETENUCLEATIONTIMESTEP_H
