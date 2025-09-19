//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalBC.h"

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the value in the node
 */
class ADArrayDirichletBC : public ADArrayNodalBC
{
public:
  static InputParameters validParams();

  ADArrayDirichletBC(const InputParameters & parameters);

protected:
  virtual ADRealEigenVector computeQpResidual() override;

  /// The value for this BC
  const RealEigenVector & _values;
};
