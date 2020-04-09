//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "DiscreteNucleationInserterBase.h"

/**
 * Returns a user defined timestep limit for the simulation step right after the
 * introduction of a new nucleus and between nucleation events to control the probability
 * of two or more nuclei appearing in one timestep.
 */
class DiscreteNucleationTimeStep : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  DiscreteNucleationTimeStep(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
  virtual PostprocessorValue getValue() override;

protected:
  /// UserObject that manages nucleus insertion and deletion
  const DiscreteNucleationInserterBase & _inserter;

  /// User specified nucleation time step
  const Real _dt_nucleation;

  /// nucleus count changes performed by the inserter
  const DiscreteNucleationInserterBase::NucleusChanges & _changes_made;

  /// total nucleation rate integrated over the entire domain
  const Real & _rate;

  /**
   * Maximum total event expectation value that is low enough so that the
   * probability for more than one * nucleation event to occurr in a single
   * timestep is below a user specified value
   */
  Real _max_lambda;
};
