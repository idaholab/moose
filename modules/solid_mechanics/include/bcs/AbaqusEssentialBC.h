//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AbaqusInputObjects.h"
#include "NodalBC.h"

class AbaqusUELStepUserObject;

/**
 * Implements Abaqus essential boundary conditions
 */
class AbaqusEssentialBC : public NodalBC
{
public:
  static InputParameters validParams();

  AbaqusEssentialBC(const InputParameters & parameters);

  void residualSetup() override;
  bool shouldApply() const override;

protected:
  Real computeQpResidual() override;

  const AbaqusUELStepUserObject & _step_uo;

  /// Abaqus ID of the current variable
  const Abaqus::AbaqusID _abaqus_var_id;

  const Real & _current_step_fraction;

  /// BC data for the current variable/step
  using NodeValueMap = std::unordered_map<Abaqus::Index, Real>;
  const NodeValueMap * _current_step_begin_solution = nullptr;
  const NodeValueMap * _current_step_begin_values = nullptr;
  const NodeValueMap * _current_step_end_values = nullptr;
};
