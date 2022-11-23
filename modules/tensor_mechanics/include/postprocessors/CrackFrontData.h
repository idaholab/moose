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
#include "CrackFrontDefinition.h"
// libMesh
#include "libmesh/node.h"

class MooseMesh;

// Forward Declarations

class CrackFrontData : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  CrackFrontData(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  const CrackFrontDefinition * const _crack_front_definition;
  const unsigned int _crack_front_point_index;
  const Node * _crack_front_node;
  MooseMesh & _mesh;
  std::string _var_name;
  const Real _scale_factor;
  MooseVariable & _field_var;
  Real _value;
};
