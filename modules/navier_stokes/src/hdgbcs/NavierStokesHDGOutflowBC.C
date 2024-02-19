//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHDGOutflowBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "NavierStokesHDGKernel.h"

registerMooseObject("NavierStokesApp", NavierStokesHDGOutflowBC);

InputParameters
NavierStokesHDGOutflowBC::validParams()
{
  auto params = HDGIntegratedBC::validParams();
  params += NavierStokesHDGInterface::validParams();
  params.addClassDescription("Implements an outflow boundary condition for use with a hybridized "
                             "discretization of the Navier-Stokes equations");
  return params;
}

NavierStokesHDGOutflowBC::NavierStokesHDGOutflowBC(const InputParameters & parameters)
  : HDGIntegratedBC(parameters), NavierStokesHDGInterface(this, this, _sys, _aux_sys, _mesh, _tid)
{
}

void
NavierStokesHDGOutflowBC::onBoundary()
{
  resizeData(*this);

  // qu, u, lm_u
  vectorFaceResidual(*this, 0, _lm_u_sol);
  vectorFaceJacobian(*this, 0, 0);
  scalarFaceResidual(*this, _vector_n_dofs, _qu_sol, _u_sol, _lm_u_sol, 0);
  scalarFaceJacobian(*this, _vector_n_dofs, 0, _vector_n_dofs, 0, 2 * _lm_n_dofs, 0, 0, _lm_n_dofs);
  lmFaceResidual(*this, 0, _qu_sol, _u_sol, _lm_u_sol, 0);
  lmFaceJacobian(*this, 0, 0, _vector_n_dofs, 0, 2 * _lm_n_dofs, 0, 0, _lm_n_dofs);
  // qv, v, lm_v
  vectorFaceResidual(*this, _vector_n_dofs + _scalar_n_dofs, _lm_v_sol);
  vectorFaceJacobian(*this, _vector_n_dofs + _scalar_n_dofs, _lm_n_dofs);
  scalarFaceResidual(*this, 2 * _vector_n_dofs + _scalar_n_dofs, _qv_sol, _v_sol, _lm_v_sol, 1);
  scalarFaceJacobian(*this,
                     2 * _vector_n_dofs + _scalar_n_dofs,
                     _vector_n_dofs + _scalar_n_dofs,
                     2 * _vector_n_dofs + _scalar_n_dofs,
                     _lm_n_dofs,
                     2 * _lm_n_dofs,
                     1,
                     0,
                     _lm_n_dofs);
  lmFaceResidual(*this, _lm_n_dofs, _qv_sol, _v_sol, _lm_v_sol, 1);
  lmFaceJacobian(*this,
                 _lm_n_dofs,
                 _vector_n_dofs + _scalar_n_dofs,
                 2 * _vector_n_dofs + _scalar_n_dofs,
                 _lm_n_dofs,
                 2 * _lm_n_dofs,
                 1,
                 0,
                 _lm_n_dofs);

  // p
  pressureFaceResidual(*this, 2 * _lm_n_dofs);
  pressureFaceJacobian(*this, 2 * _lm_n_dofs, 0, _lm_n_dofs);
}
