//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHDGVelocityDirichletBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "NavierStokesHDGKernel.h"

registerMooseObject("NavierStokesApp", NavierStokesHDGVelocityDirichletBC);

InputParameters
NavierStokesHDGVelocityDirichletBC::validParams()
{
  auto params = HDGIntegratedBC::validParams();
  params += NavierStokesHDGAssemblyHelper::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for the velocity for a "
                             "hybridized discretization of the Navier-Stokes equations");
  params.addParam<MooseFunctorName>(
      "dirichlet_u", 0, "The Dirichlet value for the x-component of velocity");
  params.addParam<MooseFunctorName>(
      "dirichlet_v", 0, "The Dirichlet value for the y-component of velocity");
  params.addParam<MooseFunctorName>(
      "dirichlet_w", 0, "The Dirichlet value for the z-component of velocity");

  return params;
}

NavierStokesHDGVelocityDirichletBC::NavierStokesHDGVelocityDirichletBC(
    const InputParameters & parameters)
  : HDGIntegratedBC(parameters),
    NavierStokesHDGAssemblyHelper(this, this, this, _sys, _aux_sys, _mesh, _tid)
{
  _dirichlet_vel[0] = &getFunctor<Real>("dirichlet_u");
  _dirichlet_vel[1] = &getFunctor<Real>("dirichlet_v");
  _dirichlet_vel[2] = &getFunctor<Real>("dirichlet_w");
}

void
NavierStokesHDGVelocityDirichletBC::onBoundary()
{
  resizeData();

  // qu, u, lm_u
  vectorDirichletResidual(0,
                          *_dirichlet_vel[0],
                          _JxW_face,
                          *_qrule_face,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point_face);
  scalarDirichletResidual(_vector_n_dofs,
                          _qu_sol,
                          _u_sol,
                          0,
                          _dirichlet_vel,
                          _JxW_face,
                          *_qrule_face,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point_face);
  scalarDirichletJacobian(
      _vector_n_dofs, 0, _vector_n_dofs, 2 * _lm_n_dofs, 0, _JxW_face, *_qrule_face, _normals);

  // qv, v, lm_v
  vectorDirichletResidual(_vector_n_dofs + _scalar_n_dofs,
                          *_dirichlet_vel[1],
                          _JxW_face,
                          *_qrule_face,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point_face);
  scalarDirichletResidual(2 * _vector_n_dofs + _scalar_n_dofs,
                          _qv_sol,
                          _v_sol,
                          1,
                          _dirichlet_vel,
                          _JxW_face,
                          *_qrule_face,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point_face);
  scalarDirichletJacobian(2 * _vector_n_dofs + _scalar_n_dofs,
                          _vector_n_dofs + _scalar_n_dofs,
                          2 * _vector_n_dofs + _scalar_n_dofs,
                          2 * _lm_n_dofs,
                          1,
                          _JxW_face,
                          *_qrule_face,
                          _normals);

  // p
  pressureDirichletResidual(2 * _lm_n_dofs,
                            _dirichlet_vel,
                            _JxW_face,
                            *_qrule_face,
                            _normals,
                            _current_elem,
                            _current_side,
                            _q_point_face);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityResidual(_lm_phi_face, _lm_u_sol, _lm_n_dofs, 0);
  createIdentityResidual(_lm_phi_face, _lm_v_sol, _lm_n_dofs, _lm_n_dofs);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, 0);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, _lm_n_dofs);
}
