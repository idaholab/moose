//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralPostprocessor.h"

class MooseMesh;

namespace libMesh
{
class Elem;
}

class ElementalVariableValue : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ElementalVariableValue(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual Real getValue() const override;
  virtual void finalize() override;
  virtual void initialSetup() override;

protected:
  /// The mesh
  const MooseMesh & _mesh;
  /// The variable being monitored
  MooseVariableField<Real> & _var;
  /// The element solution values of the monitored variable
  const VariableValue & _var_sln;
  /// The element on which the variable value is monitored
  const Elem * _element;
  /// The computed elemental variable value
  Real _value;
};
