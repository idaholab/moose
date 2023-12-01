//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class MeshDivision;

/**
 * Returns the index of the mesh division for each element / node
 */
class MeshDivisionAux : public AuxKernel
{
public:
  static InputParameters validParams();

  MeshDivisionAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Mesh division to evaluate
  const MeshDivision & _mesh_division;
  /// Number to use for the invalid value
  const int _invalid_bin_value;
};
