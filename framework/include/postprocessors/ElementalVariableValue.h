//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  virtual Real getValue() override;
  virtual void finalize() override;

protected:
  MooseMesh & _mesh;
  std::string _var_name;
  Elem * _element;
  Real _value;
};
