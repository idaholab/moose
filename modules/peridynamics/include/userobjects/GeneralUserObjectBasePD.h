//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "PeridynamicsMesh.h"

class GeneralUserObjectBasePD : public GeneralUserObject
{
public:
  static InputParameters validParams();

  GeneralUserObjectBasePD(const InputParameters & parameters);

protected:
  /// Reference to Moose mesh
  MooseMesh & _mesh;

  /// Reference to peridynamics mesh
  PeridynamicsMesh & _pdmesh;

  /// Problem dimension
  const unsigned int _dim;

  /// Reference to auxiliary system
  AuxiliarySystem & _aux;

  /// Bond status aux variable
  MooseVariable * _bond_status_var;

  /// number of nodes for a edge element
  const unsigned int _nnodes;
};
