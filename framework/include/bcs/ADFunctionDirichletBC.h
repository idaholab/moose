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

template <ComputeStage>
class ADFunctionDirichletBC;

declareADValidParams(ADFunctionDirichletBC);

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the values of a nodal variable at nodes to values specified by a function
 */
template <ComputeStage compute_stage>
class ADFunctionDirichletBC : public ADDirichletBCBase<compute_stage>
{
public:
  static InputParameters validParams();

  ADFunctionDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpValue() override;

  /// The function describing the Dirichlet condition
  const Function & _function;

  usingDirichletBCBaseMembers;
};
