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
  auto params = IntegratedBC::validParams();
  params += NavierStokesHDGAssemblyHelper::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for the velocity for a "
                             "hybridized discretization of the Navier-Stokes equations");
  params.addParam<MooseFunctorName>(
      "dirichlet_u", 0, "The Dirichlet value for the x-component of velocity");
  params.addParam<MooseFunctorName>(
      "dirichlet_v", 0, "The Dirichlet value for the y-component of velocity");
  params.addParam<MooseFunctorName>(
      "dirichlet_w", 0, "The Dirichlet value for the z-component of velocity");
  params.renameParam("variable", "u", "The x-component of velocity");
  return params;
}

NavierStokesHDGVelocityDirichletBC::NavierStokesHDGVelocityDirichletBC(
    const InputParameters & parameters)
  : IntegratedBC(parameters),
    NavierStokesHDGAssemblyHelper(this, this, this, this, _fe_problem, _sys, _mesh, _tid),
    _my_side(libMesh::invalid_uint)
{
  _dirichlet_vel[0] = &getFunctor<Real>("dirichlet_u");
  _dirichlet_vel[1] = &getFunctor<Real>("dirichlet_v");
  _dirichlet_vel[2] = &getFunctor<Real>("dirichlet_w");
}

void
NavierStokesHDGVelocityDirichletBC::initialSetup()
{
  // This check must occur after FEProblemBase::init()
  checkCoupling();
}

void
NavierStokesHDGVelocityDirichletBC::jacobianSetup()
{
  _my_elem = nullptr;
  _my_side = libMesh::invalid_uint;
}

void
NavierStokesHDGVelocityDirichletBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_my_elem != _current_elem) || (_my_side != _current_side))
  {
    computeJacobian();
    _my_elem = _current_elem;
    _my_side = _current_side;
  }
}

void
NavierStokesHDGVelocityDirichletBC::computeResidual()
{
  _grad_u_vel_re.resize(_qu_dof_indices.size());
  _u_vel_re.resize(_u_dof_indices.size());
  _lm_u_vel_re.resize(_lm_u_dof_indices.size());
  _grad_v_vel_re.resize(_qv_dof_indices.size());
  _v_vel_re.resize(_v_dof_indices.size());
  _lm_v_vel_re.resize(_lm_v_dof_indices.size());
  _p_re.resize(_p_dof_indices.size());

  // qu, u, lm_u
  vectorDirichletResidual(*_dirichlet_vel[0],
                          _JxW,
                          *_qrule,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point,
                          _grad_u_vel_re);
  scalarDirichletResidual(_qu_sol,
                          _u_sol,
                          0,
                          _dirichlet_vel,
                          _JxW,
                          *_qrule,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point,
                          _u_vel_re);

  // qv, v, lm_v
  vectorDirichletResidual(*_dirichlet_vel[1],
                          _JxW,
                          *_qrule,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point,
                          _grad_v_vel_re);
  scalarDirichletResidual(_qv_sol,
                          _v_sol,
                          1,
                          _dirichlet_vel,
                          _JxW,
                          *_qrule,
                          _normals,
                          _current_elem,
                          _current_side,
                          _q_point,
                          _v_vel_re);

  // p
  pressureDirichletResidual(
      _dirichlet_vel, _JxW, *_qrule, _normals, _current_elem, _current_side, _q_point, _p_re);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityResidual(_JxW, *_qrule, _lm_phi_face, _lm_u_sol, _lm_u_vel_re);
  createIdentityResidual(_JxW, *_qrule, _lm_phi_face, _lm_v_sol, _lm_v_vel_re);

  addResiduals(_assembly, _grad_u_vel_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _u_vel_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_u_vel_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addResiduals(_assembly, _grad_v_vel_re, _qv_dof_indices, _grad_v_var.scalingFactor());
  addResiduals(_assembly, _v_vel_re, _v_dof_indices, _v_var.scalingFactor());
  addResiduals(_assembly, _lm_v_vel_re, _lm_v_dof_indices, _v_face_var.scalingFactor());
  addResiduals(_assembly, _p_re, _p_dof_indices, _pressure_var.scalingFactor());
}

void
NavierStokesHDGVelocityDirichletBC::computeJacobian()
{
  _u_grad_u_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _u_u_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _u_p_jac.resize(_u_dof_indices.size(), _p_dof_indices.size());
  _v_grad_v_jac.resize(_v_dof_indices.size(), _qv_dof_indices.size());
  _v_v_jac.resize(_v_dof_indices.size(), _v_dof_indices.size());
  _v_p_jac.resize(_v_dof_indices.size(), _p_dof_indices.size());
  _lm_u_lm_u_jac.resize(_lm_u_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_v_lm_v_jac.resize(_lm_v_dof_indices.size(), _lm_v_dof_indices.size());

  // qu, u, lm_u
  scalarDirichletJacobian(0, _JxW, *_qrule, _normals, _u_grad_u_jac, _u_u_jac, _u_p_jac);

  // qv, v, lm_v
  scalarDirichletJacobian(1, _JxW, *_qrule, _normals, _v_grad_v_jac, _v_v_jac, _v_p_jac);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityJacobian(_JxW, *_qrule, _lm_phi_face, _lm_u_lm_u_jac);
  createIdentityJacobian(_JxW, *_qrule, _lm_phi_face, _lm_v_lm_v_jac);

  addJacobian(_assembly, _u_grad_u_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_u_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_p_jac, _u_dof_indices, _p_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _v_grad_v_jac, _v_dof_indices, _qv_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_v_jac, _v_dof_indices, _v_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_p_jac, _v_dof_indices, _p_dof_indices, _v_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_lm_u_jac, _lm_u_dof_indices, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_lm_v_jac, _lm_v_dof_indices, _lm_v_dof_indices, _v_face_var.scalingFactor());
}
