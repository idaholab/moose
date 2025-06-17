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

  /// Get current step
  unsigned int getStep() const { return _current_step; }
  const Real & getStepFraction() const { return _current_step_fraction; }

  /// get maps
  const std::unordered_map<Abaqus::Index, Real> * getBeginForces(Abaqus::AbaqusID var_id) const;
  const std::unordered_map<Abaqus::Index, Real> * getBeginSolution(Abaqus::AbaqusID var_id) const;
  const std::unordered_map<Abaqus::Index, Real> * getBeginValues(Abaqus::AbaqusID var_id) const;
  const std::unordered_map<Abaqus::Index, Real> * getEndValues(Abaqus::AbaqusID var_id) const;

  void timestepSetup() override;

protected:
  void initialize() override {}
  void execute() override {}
  void finalize() override {}

  /// Step start times
  std::vector<Real> _times;

  /// Current time
  const Real & _time;

  /// UEL Mesh object (stores BC data)
  AbaqusUELMesh & _uel_mesh;

  /// simulation steps
  const Abaqus::ObjectStore<Abaqus::Step> _steps;

  /// Map from abaqus variable ID to MooseVariable*
  std::unordered_map<Abaqus::AbaqusID, MooseVariableFieldBase *> _var_map;

  /// Current step number
  unsigned int _current_step;

  /// Get current fraction (from 0 beginning to 1 end)
  Real _current_step_fraction;

  /// Boundary conditions for the start and end of the step
  std::pair<const Abaqus::VariableValueMap<Real> *, const Abaqus::VariableValueMap<Real> *>
      _current_step_bcs;

  /// applied forces for deactivated BCs
  Abaqus::VariableValueMap<Real> _current_step_begin_forces;

  /// applied forces for deactivated BCs
  Abaqus::VariableValueMap<Real> _current_step_begin_solution;
};
