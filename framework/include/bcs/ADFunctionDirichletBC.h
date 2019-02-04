//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADFUNCTIONDIRICHLETBC_H
#define ADFUNCTIONDIRICHLETBC_H

#include "ADNodalBC.h"

template <ComputeStage>
class ADFunctionDirichletBC;

declareADValidParams(ADFunctionDirichletBC);

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the values of a nodal variable at nodes to values specified by a function
 */
template <ComputeStage compute_stage>
class ADFunctionDirichletBC : public ADNodalBC<compute_stage>
{
public:
  ADFunctionDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The function describing the Dirichlet condition
  Function & _function;

  usingNodalBCMembers;
};

#endif /* ADFUNCTIONDIRICHLETBC_H */
