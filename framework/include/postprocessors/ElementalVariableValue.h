//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTALVARIABLEVALUE_H
#define ELEMENTALVARIABLEVALUE_H

// MOOSE includes
#include "GeneralPostprocessor.h"

// Forward Declarations
class ElementalVariableValue;
class MooseMesh;

namespace libMesh
{
class Elem;
}

template <>
InputParameters validParams<ElementalVariableValue>();

class ElementalVariableValue : public GeneralPostprocessor
{
public:
  ElementalVariableValue(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override;

protected:
  MooseMesh & _mesh;
  std::string _var_name;
  Elem * _element;
};

#endif // ELEMENTALVARIABLEVALUE_H
