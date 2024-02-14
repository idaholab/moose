//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHybridizedVelocityDirichletBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "NavierStokesHybridizedKernel.h"

registerMooseObject("NavierStokesApp", NavierStokesHybridizedVelocityDirichletBC);

InputParameters
NavierStokesHybridizedVelocityDirichletBC::validParams()
{
  auto params = HybridizedIntegratedBC::validParams();
  params += NavierStokesHybridizedInterface::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for the velocity for a "
                             "hybridized discretization of the Navier-Stokes equations");
  params.addParam<FunctionName>(
      "dirichlet_u", 0, "The Dirichlet value for the x-component of velocity");
  params.addParam<FunctionName>(
      "dirichlet_v", 0, "The Dirichlet value for the y-component of velocity");
  params.addParam<FunctionName>(
      "dirichlet_w", 0, "The Dirichlet value for the z-component of velocity");

  return params;
}

NavierStokesHybridizedVelocityDirichletBC::NavierStokesHybridizedVelocityDirichletBC(
    const InputParameters & parameters)
  : HybridizedIntegratedBC(parameters),
    NavierStokesHybridizedInterface(this, this, _sys, _aux_sys, _mesh, _tid)
{
  _dirichlet_vel[0] = &getFunction("dirichlet_u");
  _dirichlet_vel[1] = &getFunction("dirichlet_v");
  _dirichlet_vel[2] = &getFunction("dirichlet_w");
}

void
NavierStokesHybridizedVelocityDirichletBC::onBoundary()
{
  resizeData(*this);

  // qu, u, lm_u
  vectorDirichletResidual(*this, 0, *_dirichlet_vel[0]);
  scalarDirichletResidual(*this, _vector_n_dofs, _qu_sol, _u_sol, 0, _dirichlet_vel);
  scalarDirichletJacobian(*this, _vector_n_dofs, 0, _vector_n_dofs, 2 * _lm_n_dofs, 0);

  // qv, v, lm_v
  vectorDirichletResidual(*this, _vector_n_dofs + _scalar_n_dofs, *_dirichlet_vel[1]);
  scalarDirichletResidual(
      *this, 2 * _vector_n_dofs + _scalar_n_dofs, _qv_sol, _v_sol, 1, _dirichlet_vel);
  scalarDirichletJacobian(*this,
                          2 * _vector_n_dofs + _scalar_n_dofs,
                          _vector_n_dofs + _scalar_n_dofs,
                          2 * _vector_n_dofs + _scalar_n_dofs,
                          2 * _lm_n_dofs,
                          1);

  // p
  pressureDirichletResidual(*this, 2 * _lm_n_dofs, _dirichlet_vel);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityResidual(_lm_phi_face, _lm_u_sol, _lm_n_dofs, 0);
  createIdentityResidual(_lm_phi_face, _lm_v_sol, _lm_n_dofs, _lm_n_dofs);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, 0);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, _lm_n_dofs);
}
