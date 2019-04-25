//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalBC.h"

template <ComputeStage>
class ADVectorFunctionDirichletBC;

declareADValidParams(ADVectorFunctionDirichletBC);

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the values of a LAGRANGE_VEC variable at nodes to values specified by functions
 */
template <ComputeStage compute_stage>
class ADVectorFunctionDirichletBC : public ADVectorNodalBC<compute_stage>
{
public:
  ADVectorFunctionDirichletBC(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue computeQpResidual() override;

  /// Optional vectorValue function
  Function * _function;

  /// x component function
  Function & _exact_x;
  /// y component function
  Function & _exact_y;
  /// z component function
  Function & _exact_z;

  /// The value for this BC
  RealVectorValue _values;

  usingVectorNodalBCMembers;
};

