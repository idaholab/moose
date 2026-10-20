//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"
#include "AbaqusInputObjects.h"
#include "AbaqusUELStepUserObject.h"

/**
 * Applies the reaction force of a deactivated Abaqus boundary condition, ramped to zero over the
 * step
 */
class AbaqusForceBC : public NodalKernel
{
public:
  static InputParameters validParams();

  AbaqusForceBC(const InputParameters & parameters);

  void residualSetup() override;

protected:
  Real computeQpResidual() override;

  const AbaqusUELStepUserObject & _step_uo;

  /// Abaqus ID of the current variable
  const Abaqus::AbaqusID _abaqus_var_id;

  const Real & _current_step_fraction;

  /// BC data for the current variable/step
  using NodeValueMap = std::unordered_map<Abaqus::Index, Real>;
  const NodeValueMap * _current_step_begin_forces = nullptr;
};
