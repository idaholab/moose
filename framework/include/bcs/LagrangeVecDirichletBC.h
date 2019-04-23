//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorNodalBC.h"

class LagrangeVecDirichletBC;

template <>
InputParameters validParams<LagrangeVecDirichletBC>();

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the value in the node
 */
class LagrangeVecDirichletBC : public VectorNodalBC
{
public:
  LagrangeVecDirichletBC(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeQpResidual() override;

  /// The value for this BC
  const RealVectorValue & _values;
};

