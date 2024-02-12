//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHybridizedOutflowBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "NavierStokesHybridizedKernel.h"

registerMooseObject("NavierStokesApp", NavierStokesHybridizedOutflowBC);

InputParameters
NavierStokesHybridizedOutflowBC::validParams()
{
  auto params = HybridizedIntegratedBC::validParams();
  params += NavierStokesHybridizedInterface::validParams();
  return params;
}

NavierStokesHybridizedOutflowBC::NavierStokesHybridizedOutflowBC(const InputParameters & parameters)
  : HybridizedIntegratedBC(parameters),
    NavierStokesHybridizedInterface(this, this, _sys, _aux_sys, _mesh, _tid),
    _JxW_face(_JxW),
    _qrule_face(_qrule)
{
}

void
NavierStokesHybridizedOutflowBC::onBoundary()
{
  resizeData(*this);

  // qu, u, lm_u
  NavierStokesHybridizedKernel::vectorFaceResidual(*this, 0, _lm_u_sol);
  NavierStokesHybridizedKernel::vectorFaceJacobian(*this, 0, 0);
  NavierStokesHybridizedKernel::scalarFaceResidual(
      *this, _vector_n_dofs, _qu_sol, _u_sol, _lm_u_sol, 0);
  NavierStokesHybridizedKernel::scalarFaceJacobian(
      *this, _vector_n_dofs, 0, _vector_n_dofs, 0, 2 * _lm_n_dofs, 0, 0, _lm_n_dofs);
  NavierStokesHybridizedKernel::lmFaceResidual(*this, 0, _qu_sol, _u_sol, _lm_u_sol, 0);
  NavierStokesHybridizedKernel::lmFaceJacobian(
      *this, 0, 0, _vector_n_dofs, 0, 2 * _lm_n_dofs, 0, 0, _lm_n_dofs);
  // qv, v, lm_v
  NavierStokesHybridizedKernel::vectorFaceResidual(
      *this, _vector_n_dofs + _scalar_n_dofs, _lm_v_sol);
  NavierStokesHybridizedKernel::vectorFaceJacobian(
      *this, _vector_n_dofs + _scalar_n_dofs, _lm_n_dofs);
  NavierStokesHybridizedKernel::scalarFaceResidual(
      *this, 2 * _vector_n_dofs + _scalar_n_dofs, _qv_sol, _v_sol, _lm_v_sol, 1);
  NavierStokesHybridizedKernel::scalarFaceJacobian(*this,
                                                   2 * _vector_n_dofs + _scalar_n_dofs,
                                                   _vector_n_dofs + _scalar_n_dofs,
                                                   2 * _vector_n_dofs + _scalar_n_dofs,
                                                   _lm_n_dofs,
                                                   2 * _lm_n_dofs,
                                                   1,
                                                   0,
                                                   _lm_n_dofs);
  NavierStokesHybridizedKernel::lmFaceResidual(*this, _lm_n_dofs, _qv_sol, _v_sol, _lm_v_sol, 1);
  NavierStokesHybridizedKernel::lmFaceJacobian(*this,
                                               _lm_n_dofs,
                                               _vector_n_dofs + _scalar_n_dofs,
                                               2 * _vector_n_dofs + _scalar_n_dofs,
                                               _lm_n_dofs,
                                               2 * _lm_n_dofs,
                                               1,
                                               0,
                                               _lm_n_dofs);

  // p
  NavierStokesHybridizedKernel::pressureFaceResidual(*this, 2 * _lm_n_dofs);
  NavierStokesHybridizedKernel::pressureFaceJacobian(*this, 2 * _lm_n_dofs, 0, _lm_n_dofs);
}
