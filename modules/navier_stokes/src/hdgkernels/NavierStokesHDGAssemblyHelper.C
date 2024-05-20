//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHDGAssemblyHelper.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "NS.h"

#include "libmesh/point.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"

using namespace libMesh;

InputParameters
NavierStokesHDGAssemblyHelper::validParams()
{
  auto params = DiffusionHDGAssemblyHelper::validParams();
  params.addRequiredParam<NonlinearVariableName>(NS::pressure, "The pressure variable.");
  params.setDocString("u", "The x-component of velocity");
  params.addRequiredParam<AuxVariableName>("v", "The y-component of velocity");
  params.addParam<AuxVariableName>("w", "The z-component of velocity");
  params.setDocString("grad_u", "The gradient of the x-component of velocity");
  params.addRequiredParam<AuxVariableName>("grad_v", "The gradient of the y-component of velocity");
  params.addParam<AuxVariableName>("grad_w", "The gradient of the z-component of velocity");
  params.setDocString("face_u", "The x-component of the face velocity");
  params.addRequiredParam<NonlinearVariableName>("face_v", "The y-component of the face velocity");
  params.addParam<NonlinearVariableName>("face_w", "The z-component of the face velocity");
  params.addParam<NonlinearVariableName>(
      "enclosure_lm",
      "For enclosed problems like the lid driven cavity this variable can be provided to remove "
      "the pressure nullspace");
  params.renameParam("diffusivity", NS::mu, "The dynamic viscosity");
  params.addRequiredParam<Real>(NS::density, "The density");
  return params;
}

NavierStokesHDGAssemblyHelper::NavierStokesHDGAssemblyHelper(const MooseObject * const moose_obj,
                                                             MaterialPropertyInterface * const mpi,
                                                             const TransientInterface * const ti,
                                                             SystemBase & nl_sys,
                                                             SystemBase & aux_sys,
                                                             const MooseMesh & mesh,
                                                             const THREAD_ID tid)
  : DiffusionHDGAssemblyHelper(moose_obj, mpi, ti, nl_sys, aux_sys, tid),
    // vars
    _v_var(aux_sys.getFieldVariable<Real>(tid, moose_obj->getParam<AuxVariableName>("v"))),
    _w_var(mesh.dimension() > 2
               ? &aux_sys.getFieldVariable<Real>(tid, moose_obj->getParam<AuxVariableName>("w"))
               : nullptr),
    _grad_v_var(aux_sys.getFieldVariable<RealVectorValue>(
        tid, moose_obj->getParam<AuxVariableName>("grad_v"))),
    _grad_w_var(mesh.dimension() > 2 ? &aux_sys.getFieldVariable<RealVectorValue>(
                                           tid, moose_obj->getParam<AuxVariableName>("grad_w"))
                                     : nullptr),
    _v_face_var(
        nl_sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("face_v"))),
    _w_face_var(mesh.dimension() > 2
                    ? &nl_sys.getFieldVariable<Real>(
                          tid, moose_obj->getParam<NonlinearVariableName>("face_w"))
                    : nullptr),
    _pressure_var(nl_sys.getFieldVariable<Real>(
        tid, moose_obj->getParam<NonlinearVariableName>(NS::pressure))),
    _enclosure_lm_var(moose_obj->isParamValid("enclosure_lm")
                          ? &nl_sys.getScalarVariable(
                                tid, moose_obj->getParam<NonlinearVariableName>("enclosure_lm"))
                          : nullptr),
    // dof indices
    _qv_dof_indices(_grad_v_var.dofIndices()),
    _v_dof_indices(_v_var.dofIndices()),
    _lm_v_dof_indices(_v_face_var.dofIndices()),
    _qw_dof_indices(_grad_w_var ? &_grad_w_var->dofIndices() : nullptr),
    _w_dof_indices(_w_var ? &_w_var->dofIndices() : nullptr),
    _lm_w_dof_indices(_w_face_var ? &_w_face_var->dofIndices() : nullptr),
    _p_dof_indices(_pressure_var.dofIndices()),
    _global_lm_dof_indices(_enclosure_lm_var ? &_enclosure_lm_var->dofIndices() : nullptr),
    // solutions
    _qv_sol(_grad_v_var.sln()),
    _v_sol(_v_var.sln()),
    _lm_v_sol(_v_face_var.sln()),
    _qw_sol(_grad_w_var ? &_grad_w_var->sln() : nullptr),
    _w_sol(_w_var ? &_w_var->sln() : nullptr),
    _lm_w_sol(_w_face_var ? &_w_face_var->sln() : nullptr),
    _p_sol(_pressure_var.sln()),
    _global_lm_dof_value(_enclosure_lm_var ? &_enclosure_lm_var->sln() : nullptr),
    _rho(moose_obj->getParam<Real>(NS::density)),
    // initialize local number of dofs
    _p_n_dofs(0),
    _global_lm_n_dofs(_enclosure_lm_var ? 1 : 0)
{
  if (mesh.dimension() > 2)
    mooseError("3D not yet implemented");
}

void
NavierStokesHDGAssemblyHelper::resizeData()
{
  _vector_n_dofs = _qu_dof_indices.size();
  _scalar_n_dofs = _u_dof_indices.size();
  _lm_n_dofs = _lm_u_dof_indices.size();
  _p_n_dofs = _p_dof_indices.size();
  libmesh_assert(_p_n_dofs == _scalar_n_dofs);

  libmesh_assert_equal_to(_vector_n_dofs, _vector_phi.size());
  libmesh_assert_equal_to(_scalar_n_dofs, _scalar_phi.size());

  _primal_size = 2 * (_vector_n_dofs + _scalar_n_dofs);
  _lm_size = 2 * _lm_n_dofs + _p_n_dofs + _global_lm_n_dofs;

  // prepare our matrix/vector data structures
  _PrimalMat.setZero(_primal_size, _primal_size);
  _PrimalVec.setZero(_primal_size);
  _LMMat.setZero(_lm_size, _lm_size);
  _LMVec.setZero(_lm_size);
  _PrimalLM.setZero(_primal_size, _lm_size);
  _LMPrimal.setZero(_lm_size, _primal_size);
}

void
NavierStokesHDGAssemblyHelper::scalarVolumeResidual(const unsigned int i_offset,
                                                    const MooseArray<Gradient> & vel_gradient,
                                                    const unsigned int vel_component,
                                                    const Moose::Functor<Real> & body_force,
                                                    const MooseArray<Real> & JxW,
                                                    const QBase & qrule,
                                                    const Elem * const current_elem,
                                                    const MooseArray<Point> & q_point)
{
  DiffusionHDGAssemblyHelper::scalarVolumeResidual(
      i_offset, vel_gradient, body_force, JxW, qrule, current_elem, q_point);

  for (const auto qp : make_range(qrule.n_points()))
  {
    const auto rho_vel_cross_vel = rhoVelCrossVelResidual(_u_sol, _v_sol, qp, vel_component);
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];

    for (const auto i : make_range(_scalar_n_dofs))
    {
      // Scalar equation dependence on pressure dofs
      _PrimalVec(i_offset + i) -= JxW[qp] * (_grad_scalar_phi[i][qp] * qp_p);

      // Scalar equation dependence on scalar dofs
      _PrimalVec(i_offset + i) -= JxW[qp] * (_grad_scalar_phi[i][qp] * rho_vel_cross_vel);
    }
  }
}

void
NavierStokesHDGAssemblyHelper::scalarVolumeJacobian(const unsigned int i_offset,
                                                    const unsigned int vel_gradient_j_offset,
                                                    const unsigned int p_j_offset,
                                                    const unsigned int vel_component,
                                                    const unsigned int u_j_offset,
                                                    const unsigned int v_j_offset,
                                                    const MooseArray<Real> & JxW,
                                                    const QBase & qrule)
{
  DiffusionHDGAssemblyHelper::scalarVolumeJacobian(i_offset, vel_gradient_j_offset, JxW, qrule);

  for (const auto i : make_range(_scalar_n_dofs))
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Scalar equation dependence on pressure dofs
      for (const auto j : make_range(_p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi[j][qp];
        _PrimalLM(i_offset + i, p_j_offset + j) -= JxW[qp] * (_grad_scalar_phi[i][qp] * p_phi);
      }

      // Scalar equation dependence on scalar dofs
      for (const auto j : make_range(_scalar_n_dofs))
      {
        // derivatives wrt 0th component of velocity
        {
          const auto rho_vel_cross_vel =
              rhoVelCrossVelJacobian(_u_sol, _v_sol, qp, vel_component, 0, _scalar_phi, j);
          _PrimalMat(i_offset + i, u_j_offset + j) -=
              JxW[qp] * (_grad_scalar_phi[i][qp] * rho_vel_cross_vel);
        }
        // derivatives wrt 1th component of velocity
        {
          const auto rho_vel_cross_vel =
              rhoVelCrossVelJacobian(_u_sol, _v_sol, qp, vel_component, 1, _scalar_phi, j);
          _PrimalMat(i_offset + i, v_j_offset + j) -=
              JxW[qp] * (_grad_scalar_phi[i][qp] * rho_vel_cross_vel);
        }
      }
    }
}

void
NavierStokesHDGAssemblyHelper::pressureVolumeResidual(
    const unsigned int i_offset,
    const unsigned int global_lm_i_offset,
    const Moose::Functor<Real> & pressure_mms_forcing_function,
    const MooseArray<Real> & JxW,
    const QBase & qrule,
    const Elem * const current_elem,
    const MooseArray<Point> & q_point)
{
  for (const auto qp : make_range(qrule.n_points()))
  {
    // Prepare forcing function
    const auto f = pressure_mms_forcing_function(
        Moose::ElemQpArg{current_elem, qp, &qrule, q_point[qp]}, _ti.determineState());

    const Gradient vel(_u_sol[qp], _v_sol[qp]);
    for (const auto i : make_range(_p_n_dofs))
    {
      _LMVec(i_offset + i) -= JxW[qp] * (_grad_scalar_phi[i][qp] * vel);

      // Pressure equation forcing function RHS
      _LMVec(i_offset + i) -= JxW[qp] * _scalar_phi[i][qp] * f;

      if (_enclosure_lm_var)
      {
        mooseAssert(
            _global_lm_dof_value->size() == 1,
            "There should only be one degree of freedom for removing the pressure nullspace");
        _LMVec(i_offset + i) -= JxW[qp] * _scalar_phi[i][qp] * (*_global_lm_dof_value)[0];
      }
    }

    if (_enclosure_lm_var)
      _LMVec(global_lm_i_offset) -= JxW[qp] * _p_sol[qp];
  }
}

void
NavierStokesHDGAssemblyHelper::pressureVolumeJacobian(const unsigned int i_offset,
                                                      const unsigned int u_j_offset,
                                                      const unsigned int v_j_offset,
                                                      const unsigned int p_j_offset,
                                                      const unsigned int global_lm_offset,
                                                      const MooseArray<Real> & JxW,
                                                      const QBase & qrule)
{
  for (const auto qp : make_range(qrule.n_points()))
  {
    for (const auto i : make_range(_p_n_dofs))
    {
      for (const auto j : make_range(_scalar_n_dofs))
      {
        {
          const Gradient phi(_scalar_phi[j][qp], 0);
          _LMPrimal(i_offset + i, u_j_offset + j) -= JxW[qp] * (_grad_scalar_phi[i][qp] * phi);
        }
        {
          const Gradient phi(0, _scalar_phi[j][qp]);
          _LMPrimal(i_offset + i, v_j_offset + j) -= JxW[qp] * (_grad_scalar_phi[i][qp] * phi);
        }
      }
      if (_enclosure_lm_var)
        _LMMat(i_offset + i, global_lm_offset) -= JxW[qp] * _scalar_phi[i][qp];
    }

    if (_enclosure_lm_var)
    {
      libmesh_assert(_scalar_n_dofs == _p_n_dofs);
      for (const auto j : make_range(_p_n_dofs))
        _LMMat(global_lm_offset, p_j_offset + j) -= JxW[qp] * _scalar_phi[j][qp];
    }
  }
}

RealVectorValue
NavierStokesHDGAssemblyHelper::rhoVelCrossVelResidual(const MooseArray<Number> & u_sol,
                                                      const MooseArray<Number> & v_sol,
                                                      const unsigned int qp,
                                                      const unsigned int vel_component)
{
  const RealVectorValue U(u_sol[qp], v_sol[qp]);
  return _rho * U * U(vel_component);
}

RealVectorValue
NavierStokesHDGAssemblyHelper::rhoVelCrossVelJacobian(const MooseArray<Number> & u_sol,
                                                      const MooseArray<Number> & v_sol,
                                                      const unsigned int qp,
                                                      const unsigned int vel_component,
                                                      const unsigned int vel_j_component,
                                                      const MooseArray<std::vector<Real>> & phi,
                                                      const unsigned int j)
{
  const RealVectorValue U(u_sol[qp], v_sol[qp]);
  RealVectorValue vector_phi;
  vector_phi(vel_j_component) = phi[j][qp];
  auto ret = vector_phi * U(vel_component);
  if (vel_component == vel_j_component)
    ret += U * phi[j][qp];
  ret *= _rho;
  return ret;
}

void
NavierStokesHDGAssemblyHelper::pressureFaceResidual(const unsigned int i_offset,
                                                    const MooseArray<Real> & JxW_face,
                                                    const QBase & qrule_face,
                                                    const MooseArray<Point> & normals)
{
  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const Gradient vel(_lm_u_sol[qp], _lm_v_sol[qp]);
    const auto vdotn = vel * normals[qp];
    for (const auto i : make_range(_p_n_dofs))
      _LMVec(i_offset + i) += JxW_face[qp] * vdotn * _scalar_phi_face[i][qp];
  }
}

void
NavierStokesHDGAssemblyHelper::pressureFaceJacobian(const unsigned int i_offset,
                                                    const unsigned int lm_u_j_offset,
                                                    const unsigned int lm_v_j_offset,
                                                    const MooseArray<Real> & JxW_face,
                                                    const QBase & qrule_face,
                                                    const MooseArray<Point> & normals)
{
  for (const auto i : make_range(_p_n_dofs))
    for (const auto j : make_range(_lm_n_dofs))
      for (const auto qp : make_range(qrule_face.n_points()))
      {
        {
          const Gradient phi(_lm_phi_face[j][qp], 0);
          _LMMat(i_offset + i, lm_u_j_offset + j) +=
              JxW_face[qp] * phi * normals[qp] * _scalar_phi_face[i][qp];
        }
        {
          const Gradient phi(0, _lm_phi_face[j][qp]);
          _LMMat(i_offset + i, lm_v_j_offset + j) +=
              JxW_face[qp] * phi * normals[qp] * _scalar_phi_face[i][qp];
        }
      }
}

void
NavierStokesHDGAssemblyHelper::scalarFaceResidual(const unsigned int i_offset,
                                                  const MooseArray<Gradient> & vector_sol,
                                                  const MooseArray<Number> & scalar_sol,
                                                  const MooseArray<Number> & lm_sol,
                                                  const unsigned int vel_component,
                                                  const MooseArray<Real> & JxW_face,
                                                  const QBase & qrule_face,
                                                  const MooseArray<Point> & normals)
{
  DiffusionHDGAssemblyHelper::scalarFaceResidual(
      i_offset, vector_sol, scalar_sol, lm_sol, JxW_face, qrule_face, normals);

  for (const auto qp : make_range(qrule_face.n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];
    const auto rho_vel_cross_vel = rhoVelCrossVelResidual(_lm_u_sol, _lm_v_sol, qp, vel_component);

    for (const auto i : make_range(_scalar_n_dofs))
    {
      // pressure
      _PrimalVec(i_offset + i) += JxW_face[qp] * _scalar_phi_face[i][qp] * (qp_p * normals[qp]);

      // lm from convection term
      _PrimalVec(i_offset + i) +=
          JxW_face[qp] * _scalar_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
    }
  }
}

void
NavierStokesHDGAssemblyHelper::scalarFaceJacobian(const unsigned int i_offset,
                                                  const unsigned int vector_j_offset,
                                                  const unsigned int scalar_j_offset,
                                                  const unsigned int lm_j_offset,
                                                  const unsigned int p_j_offset,
                                                  const unsigned int vel_component,
                                                  const unsigned int lm_u_j_offset,
                                                  const unsigned int lm_v_j_offset,
                                                  const MooseArray<Real> & JxW_face,
                                                  const QBase & qrule_face,
                                                  const MooseArray<Point> & normals)
{
  DiffusionHDGAssemblyHelper::scalarFaceJacobian(
      i_offset, vector_j_offset, scalar_j_offset, lm_j_offset, JxW_face, qrule_face, normals);

  for (const auto i : make_range(_scalar_n_dofs))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(_p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi_face[j][qp];
        // pressure
        _PrimalLM(i_offset + i, p_j_offset + j) +=
            JxW_face[qp] * _scalar_phi_face[i][qp] * (p_phi * normals[qp]);
      }

      for (const auto j : make_range(_lm_n_dofs))
      {
        //
        // from convection term
        //

        // derivatives wrt 0th component of velocity
        {
          const auto rho_vel_cross_vel =
              rhoVelCrossVelJacobian(_lm_u_sol, _lm_v_sol, qp, vel_component, 0, _lm_phi_face, j);
          _PrimalLM(i_offset + i, lm_u_j_offset + j) +=
              JxW_face[qp] * _scalar_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
        }
        // derivatives wrt 1th component of velocity
        {
          const auto rho_vel_cross_vel =
              rhoVelCrossVelJacobian(_lm_u_sol, _lm_v_sol, qp, vel_component, 1, _lm_phi_face, j);
          _PrimalLM(i_offset + i, lm_v_j_offset + j) +=
              JxW_face[qp] * _scalar_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
        }
      }
    }
}

void
NavierStokesHDGAssemblyHelper::lmFaceResidual(const unsigned int i_offset,
                                              const MooseArray<Gradient> & vector_sol,
                                              const MooseArray<Number> & scalar_sol,
                                              const MooseArray<Number> & lm_sol,
                                              const unsigned int vel_component,
                                              const MooseArray<Real> & JxW_face,
                                              const QBase & qrule_face,
                                              const MooseArray<Point> & normals,
                                              const Elem * const neigh)
{
  DiffusionHDGAssemblyHelper::lmFaceResidual(
      i_offset, vector_sol, scalar_sol, lm_sol, JxW_face, qrule_face, normals);

  for (const auto qp : make_range(qrule_face.n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];
    const auto rho_vel_cross_vel = rhoVelCrossVelResidual(_lm_u_sol, _lm_v_sol, qp, vel_component);

    for (const auto i : make_range(_lm_n_dofs))
    {
      // pressure
      _LMVec(i_offset + i) += JxW_face[qp] * _lm_phi_face[i][qp] * (qp_p * normals[qp]);

      // If we are an internal face we add the convective term. On the outflow boundary we do not
      // zero out the convection term, e.g. we are going to set q + p + tau * (u - u_hat) to zero
      if (neigh)
        // lm from convection term
        _LMVec(i_offset + i) +=
            JxW_face[qp] * _lm_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
    }
  }
}

void
NavierStokesHDGAssemblyHelper::lmFaceJacobian(const unsigned int i_offset,
                                              const unsigned int vector_j_offset,
                                              const unsigned int scalar_j_offset,
                                              const unsigned int lm_j_offset,
                                              const unsigned int p_j_offset,
                                              const unsigned int vel_component,
                                              const unsigned int lm_u_j_offset,
                                              const unsigned int lm_v_j_offset,
                                              const MooseArray<Real> & JxW_face,
                                              const QBase & qrule_face,
                                              const MooseArray<Point> & normals,
                                              const Elem * const neigh)
{
  DiffusionHDGAssemblyHelper::lmFaceJacobian(
      i_offset, vector_j_offset, scalar_j_offset, lm_j_offset, JxW_face, qrule_face, normals);

  for (const auto i : make_range(_lm_n_dofs))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(_p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi_face[j][qp];
        _LMMat(i_offset + i, p_j_offset + j) +=
            JxW_face[qp] * _lm_phi_face[i][qp] * (p_phi * normals[qp]);
      }

      for (const auto j : make_range(_lm_n_dofs))
        if (neigh)
        {
          // derivatives wrt 0th component of velocity
          {
            const auto rho_vel_cross_vel =
                rhoVelCrossVelJacobian(_lm_u_sol, _lm_v_sol, qp, vel_component, 0, _lm_phi_face, j);
            _LMMat(i_offset + i, lm_u_j_offset + j) +=
                JxW_face[qp] * _lm_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
          }
          // derivatives wrt 1th component of velocity
          {
            const auto rho_vel_cross_vel =
                rhoVelCrossVelJacobian(_lm_u_sol, _lm_v_sol, qp, vel_component, 1, _lm_phi_face, j);
            _LMMat(i_offset + i, lm_v_j_offset + j) +=
                JxW_face[qp] * _lm_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
          }
        }
    }
}

void
NavierStokesHDGAssemblyHelper::pressureDirichletResidual(
    const unsigned int i_offset,
    const std::array<const Moose::Functor<Real> *, 3> & dirichlet_vel,
    const MooseArray<Real> & JxW_face,
    const QBase & qrule_face,
    const MooseArray<Point> & normals,
    const Elem * const current_elem,
    const unsigned int current_side,
    const MooseArray<Point> & q_point_face)
{
  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const Moose::ElemSideQpArg elem_side_qp_arg{
        current_elem, current_side, qp, &qrule_face, q_point_face[qp]};
    const auto time_arg = _ti.determineState();
    const RealVectorValue dirichlet_velocity((*dirichlet_vel[0])(elem_side_qp_arg, time_arg),
                                             (*dirichlet_vel[1])(elem_side_qp_arg, time_arg),
                                             (*dirichlet_vel[2])(elem_side_qp_arg, time_arg));
    const auto vdotn = dirichlet_velocity * normals[qp];
    for (const auto i : make_range(_p_n_dofs))
      _LMVec(i_offset + i) += JxW_face[qp] * vdotn * _scalar_phi_face[i][qp];
  }
}

void
NavierStokesHDGAssemblyHelper::scalarDirichletResidual(
    const unsigned int i_offset,
    const MooseArray<Gradient> & vector_sol,
    const MooseArray<Number> & scalar_sol,
    const unsigned int vel_component,
    const std::array<const Moose::Functor<Real> *, 3> & dirichlet_vel,
    const MooseArray<Real> & JxW_face,
    const QBase & qrule_face,
    const MooseArray<Point> & normals,
    const Elem * const current_elem,
    const unsigned int current_side,
    const MooseArray<Point> & q_point_face)
{
  DiffusionHDGAssemblyHelper::scalarDirichletResidual(i_offset,
                                                      vector_sol,
                                                      scalar_sol,
                                                      *dirichlet_vel[vel_component],
                                                      JxW_face,
                                                      qrule_face,
                                                      normals,
                                                      current_elem,
                                                      current_side,
                                                      q_point_face);

  for (const auto qp : make_range(qrule_face.n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];

    const Moose::ElemSideQpArg elem_side_qp_arg{
        current_elem, current_side, qp, &qrule_face, q_point_face[qp]};
    const auto time_arg = _ti.determineState();
    const RealVectorValue dirichlet_velocity((*dirichlet_vel[0])(elem_side_qp_arg, time_arg),
                                             (*dirichlet_vel[1])(elem_side_qp_arg, time_arg),
                                             (*dirichlet_vel[2])(elem_side_qp_arg, time_arg));
    const auto scalar_value = dirichlet_velocity(vel_component);

    for (const auto i : make_range(_scalar_n_dofs))
    {
      // pressure
      _PrimalVec(i_offset + i) += JxW_face[qp] * _scalar_phi_face[i][qp] * (qp_p * normals[qp]);

      // dirichlet lm from advection term
      _PrimalVec(i_offset + i) += JxW_face[qp] * _scalar_phi_face[i][qp] *
                                  (_rho * dirichlet_velocity * normals[qp]) * scalar_value;
    }
  }
}

void
NavierStokesHDGAssemblyHelper::scalarDirichletJacobian(const unsigned int i_offset,
                                                       const unsigned int vector_j_offset,
                                                       const unsigned int scalar_j_offset,
                                                       const unsigned int p_j_offset,
                                                       const unsigned int vel_component,
                                                       const MooseArray<Real> & JxW_face,
                                                       const QBase & qrule_face,
                                                       const MooseArray<Point> & normals)
{
  DiffusionHDGAssemblyHelper::scalarDirichletJacobian(
      i_offset, vector_j_offset, scalar_j_offset, JxW_face, qrule_face, normals);

  for (const auto i : make_range(_scalar_n_dofs))
    for (const auto j : make_range(_p_n_dofs))
      for (const auto qp : make_range(qrule_face.n_points()))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi_face[j][qp];
        // pressure
        _PrimalLM(i_offset + i, p_j_offset + j) +=
            JxW_face[qp] * _scalar_phi_face[i][qp] * (p_phi * normals[qp]);
      }
}

std::set<const MooseVariableBase *>
NavierStokesHDGAssemblyHelper::variables() const
{
  auto ret = DiffusionHDGAssemblyHelper::variables();
  ret.insert(&_v_var);
  ret.insert(&_grad_v_var);
  ret.insert(&_v_face_var);
  ret.insert(&_pressure_var);
  if (_w_var)
  {
    ret.insert(_w_var);
    ret.insert(_grad_w_var);
    ret.insert(_w_face_var);
  }
  if (_enclosure_lm_var)
    ret.insert(_enclosure_lm_var);

  return ret;
}
