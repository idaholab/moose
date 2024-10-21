//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirectDirichletBCBase.h"
#include "NonlinearSystemBase.h"

InputParameters
DirectDirichletBCBase::validParams()
{
  InputParameters params = NodalBC::validParams();
  return params;
}

DirectDirichletBCBase::DirectDirichletBCBase(const InputParameters & parameters)
  : NodalBC(parameters),
    _mass_diag(_sys.getVector("mass_matrix_diag")),
    _u_old(_var.nodalValueOld()),
    _u_dot_old(_var.nodalValueDotOld())
{
}

Real
DirectDirichletBCBase::computeQpResidual()
{
  // Get dof for current var
  const auto dofnum = _variable->nodalDofIndex();

  // Compute residual to enforce BC
  // This is the force required to enforce the BC in a central difference scheme
  Real avg_dt = (_dt + _dt_old) / 2;
  Real resid = (computeQpValue() - _u_old) / (avg_dt * _dt) - (_u_dot_old) / avg_dt;
  resid *= -_mass_diag(dofnum);

  return resid;
}
