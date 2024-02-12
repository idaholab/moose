//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHybridizedKernel.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", NavierStokesHybridizedKernel);

InputParameters
NavierStokesHybridizedKernel::validParams()
{
  auto params = HybridizedKernel::validParams();
  params += NavierStokesHybridizedInterface::validParams();
  params.addParam<FunctionName>(
      "body_force_x", 0, "Body force for the momentum equation in the x-direction");
  params.addParam<FunctionName>(
      "body_force_y", 0, "Body force for the momentum equation in the y-direction");
  params.addParam<FunctionName>(
      "body_force_z", 0, "Body force for the momentum equation in the z-direction");
  params.addParam<FunctionName>(
      "pressure_mms_forcing_function",
      0,
      "A forcing function for the pressure (mass) equation for conducting MMS studies");
  params.addClassDescription("Implements the steady incompressible Navier-Stokes equations for a "
                             "hybridized discretization");

  return params;
}

NavierStokesHybridizedKernel::NavierStokesHybridizedKernel(const InputParameters & parameters)
  : HybridizedKernel(parameters),
    NavierStokesHybridizedInterface(this, this, _sys, _aux_sys, _mesh, _tid),
    // body forces
    _body_force_x(getFunction("body_force_x")),
    _body_force_y(getFunction("body_force_y")),
    _body_force_z(getFunction("body_force_z")),
    _pressure_mms_forcing_function(getFunction("pressure_mms_forcing_function"))
{
  _body_forces.push_back(&_body_force_x);
  _body_forces.push_back(&_body_force_y);
  _body_forces.push_back(&_body_force_z);
}

void
NavierStokesHybridizedKernel::onElement()
{
  resizeData(*this);

  // Populate LM dof indices
  _lm_dof_indices = _lm_u_dof_indices;
  _lm_dof_indices.insert(_lm_dof_indices.end(), _lm_v_dof_indices.begin(), _lm_v_dof_indices.end());
  _lm_dof_indices.insert(_lm_dof_indices.end(), _p_dof_indices.begin(), _p_dof_indices.end());
  if (_global_lm_dof_indices)
    _lm_dof_indices.insert(
        _lm_dof_indices.end(), _global_lm_dof_indices->begin(), _global_lm_dof_indices->end());

  // Populate primal dof indices if we are computing the primal increment
  if (!computingGlobalData())
  {
    _primal_dof_indices = _qu_dof_indices;
    auto append = [this](const auto & dofs)
    { _primal_dof_indices.insert(_primal_dof_indices.end(), dofs.begin(), dofs.end()); };
    append(_u_dof_indices);
    append(_qv_dof_indices);
    append(_v_dof_indices);
  }

  // qu and u
  vectorVolumeResidual(0, _qu_sol, _u_sol);
  scalarVolumeResidual(_vector_n_dofs, _qu_sol, 0, _sigma_u);
  vectorVolumeJacobian(0, 0, _vector_n_dofs);
  scalarVolumeJacobian(
      _vector_n_dofs, 0, 2 * _lm_n_dofs, 0, _vector_n_dofs, 2 * _vector_n_dofs + _scalar_n_dofs);

  // qv and v
  vectorVolumeResidual(_vector_n_dofs + _scalar_n_dofs, _qv_sol, _v_sol);
  scalarVolumeResidual(2 * _vector_n_dofs + _scalar_n_dofs, _qv_sol, 1, _sigma_v);
  vectorVolumeJacobian(_vector_n_dofs + _scalar_n_dofs,
                       _vector_n_dofs + _scalar_n_dofs,
                       2 * _vector_n_dofs + _scalar_n_dofs);
  scalarVolumeJacobian(2 * _vector_n_dofs + _scalar_n_dofs,
                       _vector_n_dofs + _scalar_n_dofs,
                       2 * _lm_n_dofs,
                       1,
                       _vector_n_dofs,
                       2 * _vector_n_dofs + _scalar_n_dofs);

  // p
  pressureVolumeResidual(2 * _lm_n_dofs, 2 * _lm_n_dofs + _p_n_dofs);
  pressureVolumeJacobian(2 * _lm_n_dofs,
                         _vector_n_dofs,
                         2 * _vector_n_dofs + _scalar_n_dofs,
                         2 * _lm_n_dofs,
                         2 * _lm_n_dofs + _p_n_dofs);
}

void
NavierStokesHybridizedKernel::onInternalSide()
{
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

void
NavierStokesHybridizedKernel::computeStress(const MooseArray<Gradient> & vel_gradient,
                                            const unsigned int vel_component,
                                            std::vector<Gradient> & sigma)
{
  sigma.resize(_qrule->n_points());
  for (const auto qp : make_range(_qrule->n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];
    sigma[qp] = _nu[qp] * vel_gradient[qp] - qp_p;
  }
}

void
NavierStokesHybridizedKernel::vectorVolumeResidual(const unsigned int i_offset,
                                                   const MooseArray<Gradient> & vector_sol,
                                                   const MooseArray<Number> & scalar_sol)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : make_range(_vector_n_dofs))
    {
      // Vector equation dependence on vector dofs
      _PrimalVec(i_offset + i) += _JxW[qp] * (_vector_phi[i][qp] * vector_sol[qp]);

      // Vector equation dependence on scalar dofs
      _PrimalVec(i_offset + i) += _JxW[qp] * (_div_vector_phi[i][qp] * scalar_sol[qp]);
    }
}

void
NavierStokesHybridizedKernel::vectorVolumeJacobian(const unsigned int i_offset,
                                                   const unsigned int vector_j_offset,
                                                   const unsigned int scalar_j_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : make_range(_vector_n_dofs))
    {
      // Vector equation dependence on vector dofs
      for (const auto j : make_range(_vector_n_dofs))
        _PrimalMat(i_offset + i, vector_j_offset + j) +=
            _JxW[qp] * (_vector_phi[i][qp] * _vector_phi[j][qp]);

      // Vector equation dependence on scalar dofs
      for (const auto j : make_range(_scalar_n_dofs))
        _PrimalMat(i_offset + i, scalar_j_offset + j) +=
            _JxW[qp] * (_div_vector_phi[i][qp] * _scalar_phi[j][qp]);
    }
}

void
NavierStokesHybridizedKernel::scalarVolumeResidual(const unsigned int i_offset,
                                                   const MooseArray<Gradient> & vel_gradient,
                                                   const unsigned int vel_component,
                                                   std::vector<Gradient> & sigma)
{
  computeStress(vel_gradient, vel_component, sigma);
  const auto & body_force = *_body_forces[vel_component];
  for (const auto qp : make_range(_qrule->n_points()))
  {
    const auto vel_cross_vel = velCrossVelResidual(_u_sol, _v_sol, qp, vel_component);

    // Evaluate body force
    const auto f = body_force.value(_t, _q_point[qp]);

    for (const auto i : make_range(_scalar_n_dofs))
    {
      // Scalar equation dependence on vector and pressure dofs
      _PrimalVec(i_offset + i) += _JxW[qp] * (_grad_scalar_phi[i][qp] * sigma[qp]);

      // Scalar equation dependence on scalar dofs
      _PrimalVec(i_offset + i) -= _JxW[qp] * (_grad_scalar_phi[i][qp] * vel_cross_vel);

      // Scalar equation RHS
      _PrimalVec(i_offset + i) -= _JxW[qp] * _scalar_phi[i][qp] * f;
    }
  }
}

void
NavierStokesHybridizedKernel::scalarVolumeJacobian(const unsigned int i_offset,
                                                   const unsigned int vel_gradient_j_offset,
                                                   const unsigned int p_j_offset,
                                                   const unsigned int vel_component,
                                                   const unsigned int u_j_offset,
                                                   const unsigned int v_j_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : make_range(_scalar_n_dofs))
    {
      // Scalar equation dependence on vector dofs
      for (const auto j : make_range(_vector_n_dofs))
        _PrimalMat(i_offset + i, vel_gradient_j_offset + j) +=
            _JxW[qp] * _nu[qp] * (_grad_scalar_phi[i][qp] * _vector_phi[j][qp]);

      // Scalar equation dependence on pressure dofs
      for (const auto j : make_range(_p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi[j][qp];
        _PrimalLM(i_offset + i, p_j_offset + j) -= _JxW[qp] * (_grad_scalar_phi[i][qp] * p_phi);
      }

      // Scalar equation dependence on scalar dofs
      for (const auto j : make_range(_scalar_n_dofs))
      {
        // derivatives wrt 0th component
        {
          const auto vel_cross_vel =
              velCrossVelJacobian(_u_sol, _v_sol, qp, vel_component, 0, _scalar_phi, j);
          _PrimalMat(i_offset + i, u_j_offset + j) -=
              _JxW[qp] * (_grad_scalar_phi[i][qp] * vel_cross_vel);
        }
        // derivatives wrt 1th component
        {
          const auto vel_cross_vel =
              velCrossVelJacobian(_u_sol, _v_sol, qp, vel_component, 1, _scalar_phi, j);
          _PrimalMat(i_offset + i, v_j_offset + j) -=
              _JxW[qp] * (_grad_scalar_phi[i][qp] * vel_cross_vel);
        }
      }
    }
}

void
NavierStokesHybridizedKernel::pressureVolumeResidual(const unsigned int i_offset,
                                                     const unsigned int global_lm_i_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    // Prepare forcing function
    const auto f = _pressure_mms_forcing_function.value(_t, _q_point[qp]);

    const Gradient vel(_u_sol[qp], _v_sol[qp]);
    for (const auto i : make_range(_p_n_dofs))
    {
      _LMVec(i_offset + i) -= _JxW[qp] * (_grad_scalar_phi[i][qp] * vel);

      // Pressure equation forcing function RHS
      _LMVec(i_offset + i) -= _JxW[qp] * _scalar_phi[i][qp] * f;

      if (_enclosure_lm_var)
      {
        mooseAssert(
            _global_lm_dof_value->size() == 1,
            "There should only be one degree of freedom for removing the pressure nullspace");
        _LMVec(i_offset + i) -= _JxW[qp] * _scalar_phi[i][qp] * (*_global_lm_dof_value)[0];
      }
    }

    if (_enclosure_lm_var)
      _LMVec(global_lm_i_offset) -= _JxW[qp] * _p_sol[qp];
  }
}

void
NavierStokesHybridizedKernel::pressureVolumeJacobian(const unsigned int i_offset,
                                                     const unsigned int u_j_offset,
                                                     const unsigned int v_j_offset,
                                                     const unsigned int p_j_offset,
                                                     const unsigned int global_lm_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    for (const auto i : make_range(_p_n_dofs))
    {
      for (const auto j : make_range(_scalar_n_dofs))
      {
        {
          const Gradient phi(_scalar_phi[j][qp], 0);
          _LMPrimal(i_offset + i, u_j_offset + j) -= _JxW[qp] * (_grad_scalar_phi[i][qp] * phi);
        }
        {
          const Gradient phi(0, _scalar_phi[j][qp]);
          _LMPrimal(i_offset + i, v_j_offset + j) -= _JxW[qp] * (_grad_scalar_phi[i][qp] * phi);
        }
      }
      if (_enclosure_lm_var)
        _LMMat(i_offset + i, global_lm_offset) -= _JxW[qp] * _scalar_phi[i][qp];
    }

    if (_enclosure_lm_var)
    {
      libmesh_assert(_scalar_n_dofs == _p_n_dofs);
      for (const auto j : make_range(_p_n_dofs))
        _LMMat(global_lm_offset, p_j_offset + j) -= _JxW[qp] * _scalar_phi[j][qp];
    }
  }
}

RealVectorValue
NavierStokesHybridizedKernel::velCrossVelResidual(const MooseArray<Number> & u_sol,
                                                  const MooseArray<Number> & v_sol,
                                                  const unsigned int qp,
                                                  const unsigned int vel_component)
{
  const RealVectorValue U(u_sol[qp], v_sol[qp]);
  return U * U(vel_component);
}

RealVectorValue
NavierStokesHybridizedKernel::velCrossVelJacobian(const MooseArray<Number> & u_sol,
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
  return ret;
}
