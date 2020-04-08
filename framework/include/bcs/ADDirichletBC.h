//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDirichletBCBase.h"

#define usingDirichletBCMembers                                                                    \
  usingDirichletBCBaseMembers;                                                                     \
  using ADDirichletBC<compute_stage>::computeQpValue

template <ComputeStage>
class ADDirichletBC;

declareADValidParams(ADDirichletBC);

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the values of a nodal variable at nodes
 */
template <ComputeStage compute_stage>
class ADDirichletBC : public ADDirichletBCBase<compute_stage>
{
public:
  static InputParameters validParams();

  ADDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpValue() override;

  /// The value for this BC
  const Real & _value;

  usingDirichletBCBaseMembers;
};
