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
#include <unordered_map>

class AbaqusUELMesh;
class AbaqusUELStepUserObject;

/**
 * Precomputes distributed load arrays per element at the beginning of each timestep
 * by interpolating between begin/end DLOAD maps from the step user object.
 */
class AbaqusDLoadInterpolator : public GeneralUserObject
{
public:
  static InputParameters validParams();

  AbaqusDLoadInterpolator(const InputParameters & parameters);

  void timestepSetup() override;

  void initialize() override {}
  void execute() override {}
  void finalize() override {}

  /// Get JDLTYP array for element (nullptr if none)
  const std::vector<int> * getTypes(Abaqus::Index elem_index) const;
  /// Get ADLMAG array for element (nullptr if none)
  const std::vector<Real> * getMagnitudes(Abaqus::Index elem_index) const;

protected:
  AbaqusUELMesh & _uel_mesh;
  const AbaqusUELStepUserObject & _step_uo;

  std::unordered_map<Abaqus::Index, std::vector<int>> _types;
  std::unordered_map<Abaqus::Index, std::vector<Real>> _magnitudes;
};
