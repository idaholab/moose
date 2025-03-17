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
  auto params = IntegratedBC::validParams();
  params += NavierStokesHDGAssemblyHelper::validParams();
  params.addClassDescription("Implements an outflow boundary condition for use with a hybridized "
                             "discretization of the incompressible Navier-Stokes equations");
  params.renameParam("variable", "u", "The x-component of velocity");
  return params;
}

NavierStokesHDGOutflowBC::NavierStokesHDGOutflowBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    NavierStokesHDGAssemblyHelper(this, this, this, this, _fe_problem, _sys, _mesh, _tid),
    _my_side(libMesh::invalid_uint)
{
}

void
NavierStokesHDGOutflowBC::initialSetup()
{
  checkCoupling();
}

void
NavierStokesHDGOutflowBC::jacobianSetup()
{
  _my_elem = nullptr;
  _my_side = libMesh::invalid_uint;
}

void
NavierStokesHDGOutflowBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_my_elem != _current_elem) || (_my_side != _current_side))
  {
    computeJacobian();
    _my_elem = _current_elem;
    _my_side = _current_side;
  }
}

void
NavierStokesHDGOutflowBC::computeResidual()
{
  const Elem * const neigh = _current_elem->neighbor_ptr(_current_side);

  _grad_u_vel_re.resize(_qu_dof_indices.size());
  _u_vel_re.resize(_u_dof_indices.size());
  _lm_u_vel_re.resize(_lm_u_dof_indices.size());
  _grad_v_vel_re.resize(_qv_dof_indices.size());
  _v_vel_re.resize(_v_dof_indices.size());
  _lm_v_vel_re.resize(_lm_v_dof_indices.size());
  _p_re.resize(_p_dof_indices.size());

  // qu, u, lm_u
  vectorFaceResidual(_lm_u_sol, _JxW, *_qrule, _normals, _grad_u_vel_re);
  scalarFaceResidual(_qu_sol, _u_sol, _lm_u_sol, 0, _JxW, *_qrule, _normals, _u_vel_re);
  lmFaceResidual(_qu_sol, _u_sol, _lm_u_sol, 0, _JxW, *_qrule, _normals, neigh, _lm_u_vel_re);

  // qv, v, lm_v
  vectorFaceResidual(_lm_v_sol, _JxW, *_qrule, _normals, _grad_v_vel_re);
  scalarFaceResidual(_qv_sol, _v_sol, _lm_v_sol, 1, _JxW, *_qrule, _normals, _v_vel_re);
  lmFaceResidual(_qv_sol, _v_sol, _lm_v_sol, 1, _JxW, *_qrule, _normals, neigh, _lm_v_vel_re);

  // p
  pressureFaceResidual(_JxW, *_qrule, _normals, _p_re);

  addResiduals(_assembly, _grad_u_vel_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _u_vel_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_u_vel_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addResiduals(_assembly, _grad_v_vel_re, _qv_dof_indices, _grad_v_var.scalingFactor());
  addResiduals(_assembly, _v_vel_re, _v_dof_indices, _v_var.scalingFactor());
  addResiduals(_assembly, _lm_v_vel_re, _lm_v_dof_indices, _v_face_var.scalingFactor());
  addResiduals(_assembly, _p_re, _p_dof_indices, _pressure_var.scalingFactor());
}

void
NavierStokesHDGOutflowBC::computeJacobian()
{
  const Elem * const neigh = _current_elem->neighbor_ptr(_current_side);

  _grad_u_lm_u_jac.resize(_qu_dof_indices.size(), _lm_u_dof_indices.size());
  _u_grad_u_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _u_u_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _u_lm_u_jac.resize(_u_dof_indices.size(), _lm_u_dof_indices.size());
  _u_lm_v_jac.resize(_u_dof_indices.size(), _lm_v_dof_indices.size());
  _u_p_jac.resize(_u_dof_indices.size(), _p_dof_indices.size());
  _lm_u_grad_u_jac.resize(_lm_u_dof_indices.size(), _qu_dof_indices.size());
  _lm_u_u_jac.resize(_lm_u_dof_indices.size(), _u_dof_indices.size());
  _lm_u_lm_u_jac.resize(_lm_u_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_u_lm_v_jac.resize(_lm_u_dof_indices.size(), _lm_v_dof_indices.size());
  _lm_u_p_jac.resize(_lm_u_dof_indices.size(), _p_dof_indices.size());
  _grad_v_lm_v_jac.resize(_qv_dof_indices.size(), _lm_v_dof_indices.size());
  _v_grad_v_jac.resize(_v_dof_indices.size(), _qv_dof_indices.size());
  _v_v_jac.resize(_v_dof_indices.size(), _v_dof_indices.size());
  _v_lm_u_jac.resize(_v_dof_indices.size(), _lm_u_dof_indices.size());
  _v_lm_v_jac.resize(_v_dof_indices.size(), _lm_v_dof_indices.size());
  _v_p_jac.resize(_v_dof_indices.size(), _p_dof_indices.size());
  _lm_v_grad_v_jac.resize(_lm_v_dof_indices.size(), _qv_dof_indices.size());
  _lm_v_v_jac.resize(_lm_v_dof_indices.size(), _v_dof_indices.size());
  _lm_v_lm_u_jac.resize(_lm_v_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_v_lm_v_jac.resize(_lm_v_dof_indices.size(), _lm_v_dof_indices.size());
  _lm_v_p_jac.resize(_lm_v_dof_indices.size(), _p_dof_indices.size());
  _p_lm_u_jac.resize(_p_dof_indices.size(), _lm_u_dof_indices.size());
  _p_lm_v_jac.resize(_p_dof_indices.size(), _lm_v_dof_indices.size());

  // qu, u, lm_u
  vectorFaceJacobian(_JxW, *_qrule, _normals, _grad_u_lm_u_jac);
  scalarFaceJacobian(0,
                     _JxW,
                     *_qrule,
                     _normals,
                     _u_grad_u_jac,
                     _u_u_jac,
                     _u_lm_u_jac,
                     _u_p_jac,
                     _u_lm_u_jac,
                     _u_lm_v_jac);
  lmFaceJacobian(0,
                 _JxW,
                 *_qrule,
                 _normals,
                 neigh,
                 _lm_u_grad_u_jac,
                 _lm_u_u_jac,
                 _lm_u_lm_u_jac,
                 _lm_u_p_jac,
                 _lm_u_lm_u_jac,
                 _lm_u_lm_v_jac);

  // qv, v, lm_v
  vectorFaceJacobian(_JxW, *_qrule, _normals, _grad_v_lm_v_jac);
  scalarFaceJacobian(1,
                     _JxW,
                     *_qrule,
                     _normals,
                     _v_grad_v_jac,
                     _v_v_jac,
                     _v_lm_v_jac,
                     _v_p_jac,
                     _v_lm_u_jac,
                     _v_lm_v_jac);
  lmFaceJacobian(1,
                 _JxW,
                 *_qrule,
                 _normals,
                 neigh,
                 _lm_v_grad_v_jac,
                 _lm_v_v_jac,
                 _lm_v_lm_v_jac,
                 _lm_v_p_jac,
                 _lm_v_lm_u_jac,
                 _lm_v_lm_v_jac);

  // p
  pressureFaceJacobian(_JxW, *_qrule, _normals, _p_lm_u_jac, _p_lm_v_jac);

  addJacobian(
      _assembly, _grad_u_lm_u_jac, _qu_dof_indices, _lm_u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(_assembly, _u_grad_u_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_u_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_lm_u_jac, _u_dof_indices, _lm_u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_lm_v_jac, _u_dof_indices, _lm_v_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_p_jac, _u_dof_indices, _p_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_grad_u_jac, _lm_u_dof_indices, _qu_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_u_jac, _lm_u_dof_indices, _u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_lm_u_jac, _lm_u_dof_indices, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_lm_v_jac, _lm_u_dof_indices, _lm_v_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_p_jac, _lm_u_dof_indices, _p_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _grad_v_lm_v_jac, _qv_dof_indices, _lm_v_dof_indices, _grad_v_var.scalingFactor());
  addJacobian(_assembly, _v_grad_v_jac, _v_dof_indices, _qv_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_v_jac, _v_dof_indices, _v_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_lm_u_jac, _v_dof_indices, _lm_u_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_lm_v_jac, _v_dof_indices, _lm_v_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_p_jac, _v_dof_indices, _p_dof_indices, _v_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_grad_v_jac, _lm_v_dof_indices, _qv_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_v_jac, _lm_v_dof_indices, _v_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_lm_u_jac, _lm_v_dof_indices, _lm_u_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_lm_v_jac, _lm_v_dof_indices, _lm_v_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_p_jac, _lm_v_dof_indices, _p_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _p_lm_u_jac, _p_dof_indices, _lm_u_dof_indices, _pressure_var.scalingFactor());
  addJacobian(
      _assembly, _p_lm_v_jac, _p_dof_indices, _lm_v_dof_indices, _pressure_var.scalingFactor());
}
