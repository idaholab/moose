//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "PeridynamicsMesh.h"

/**
 * Base element userobject class for peridynamics
 */
class ElementUserObjectBasePD : public ElementUserObject
{
public:
  static InputParameters validParams();

  ElementUserObjectBasePD(const InputParameters & parameters);

protected:
  /// Bond status aux variable
  MooseVariable * _bond_status_var;

  /// Reference to auxiliary system
  AuxiliarySystem & _aux;

  /// Reference to Peridynamic mesh
  PeridynamicsMesh & _pdmesh;

  /// number of nodes for a edge element
  const unsigned int _nnodes;
};
