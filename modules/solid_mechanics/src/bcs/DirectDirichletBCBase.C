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
  params.addParam<TagName>("mass_matrix_tag", "mass", "The tag for the mass matrix");

  return params;
}

DirectDirichletBCBase::DirectDirichletBCBase(const InputParameters & parameters)
  : NodalBC(parameters), _mass_matrix(nullptr), _u_old(_var.slnOld()), _u_dot_old(_var.uDotOld())
{
}

void
DirectDirichletBCBase::initialSetup()
{
  _mass_matrix = &_sys.system().get_matrix("System Matrix");
}

Real
DirectDirichletBCBase::computeQpResidual()
{
  // Get dof for current var
  const auto dofnum =
      _current_node->dof_number(_sys.number(), variable().number(), _sys.dofMap().sys_number());

  // // Get lumped component of mass matrix
  auto & diag = _sys.getVector("mass_matrix_diag");
  Real mass_q = diag(dofnum);

  // Compute residual to enforce BC
  // This is the force required to enforce the BC in a central difference scheme
  Real avg_dt = (_dt + _dt_old) / 2;
  Real resid = (computeQpValue() - _u_old[_qp]) / (avg_dt * _dt) - (_u_dot_old[_qp]) / avg_dt;
  resid *= -mass_q;

  return resid;
}
