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

class MooseMesh;

namespace libMesh
{
class Node;
}

/**
 * Sums a nodal value across all processors and multiplies the result
 * by a scale factor.
 */
class NodalVariableValue : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NodalVariableValue(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void finalize() override;
  void initialSetup() override;

protected:
  MooseMesh & _mesh;
  std::string _var_name;
  Node * _node_ptr;
  const Real _scale_factor;
  Real _value;
};
