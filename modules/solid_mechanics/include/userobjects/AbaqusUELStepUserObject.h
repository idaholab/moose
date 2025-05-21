//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "AbaqusInputObjects.h"
class AbaqusUELMesh;

/**
 * User object that provides simulation steps given user input
 */
class AbaqusUELStepUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  AbaqusUELStepUserObject(const InputParameters & parameters);

  /// Get initial dt
  Real getInitialDt() const { return _steps[_current_step]._dt; }

  /// Get crrent step
  std::size_t getStep() const { return _current_step; }

protected:
  void initialize() override;
  void execute() override;
  void finalize() override;

  /// Step start times
  std::vector<Real> _times;

  /// Current time
  const Real & _time;

  /// Current step number
  std::size_t _current_step;

  /// UEL Mesh object (stores BC data)
  AbaqusUELMesh & _uel_mesh;

  /// simulation steps
  const Abaqus::ObjectStore<Abaqus::Step> _steps;

  /// Map from abaqus variable ID to MooseVariable*
  std::unordered_map<Abaqus::AbaqusID, MooseVariableFieldBase*> _var_map;

  /// beginning and end of step values
  Abaqus::VariableValueMap<std::pair<Real, Real>> _bc_values;

  /// applied forces for deactivated BCs
  Abaqus::VariableValueMap<Real> _bc_forces;

  /// tagged resodual with concentrated forces
  const NumericVector<Number> & _concentrated_forces;
};
