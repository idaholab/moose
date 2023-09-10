//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * This object adds the sparsity dependence of the surface displacement degrees of
 * freedom on surface velocity degrees of freedom introduced by the nodal boundary
 * condition INSADDisplaceBoundaryBC. This sparsity must be added before
 * nodal boundary conditions are executed because the Jacobian matrix is assembled
 * prior to nodal boundary condition execution. At that time, if there is unused
 * sparsity in the matrix it is removed by PETSc. Hence the use of this object to
 * prevent new nonzero allocations during execution of INSADDisplaceBoundaryBC.
 */
class INSADDummyDisplaceBoundaryIntegratedBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  INSADDummyDisplaceBoundaryIntegratedBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The velocity vector
  const ADVectorVariableValue & _velocity;

  /// What component of velocity/displacement this object is acting on
  const unsigned short _component;
};
