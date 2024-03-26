//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesHDGKernel.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", NavierStokesHDGKernel);

InputParameters
NavierStokesHDGKernel::validParams()
{
  auto params = HDGKernel::validParams();
  params += NavierStokesHDGAssemblyHelper::validParams();
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

NavierStokesHDGKernel::NavierStokesHDGKernel(const InputParameters & parameters)
  : HDGKernel(parameters),
    NavierStokesHDGAssemblyHelper(this, this, _sys, _aux_sys, _mesh, _tid),
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
NavierStokesHDGKernel::onElement()
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
  vectorVolumeResidual(*this, 0, _qu_sol, _u_sol);
  scalarVolumeResidual(*this, _vector_n_dofs, _qu_sol, 0, _body_force_x);
  vectorVolumeJacobian(*this, 0, 0, _vector_n_dofs);
  scalarVolumeJacobian(*this,
                       _vector_n_dofs,
                       0,
                       2 * _lm_n_dofs,
                       0,
                       _vector_n_dofs,
                       2 * _vector_n_dofs + _scalar_n_dofs);

  // qv and v
  vectorVolumeResidual(*this, _vector_n_dofs + _scalar_n_dofs, _qv_sol, _v_sol);
  scalarVolumeResidual(*this, 2 * _vector_n_dofs + _scalar_n_dofs, _qv_sol, 1, _body_force_y);
  vectorVolumeJacobian(*this,
                       _vector_n_dofs + _scalar_n_dofs,
                       _vector_n_dofs + _scalar_n_dofs,
                       2 * _vector_n_dofs + _scalar_n_dofs);
  scalarVolumeJacobian(*this,
                       2 * _vector_n_dofs + _scalar_n_dofs,
                       _vector_n_dofs + _scalar_n_dofs,
                       2 * _lm_n_dofs,
                       1,
                       _vector_n_dofs,
                       2 * _vector_n_dofs + _scalar_n_dofs);

  // p
  pressureVolumeResidual(
      *this, 2 * _lm_n_dofs, 2 * _lm_n_dofs + _p_n_dofs, _pressure_mms_forcing_function);
  pressureVolumeJacobian(*this,
                         2 * _lm_n_dofs,
                         _vector_n_dofs,
                         2 * _vector_n_dofs + _scalar_n_dofs,
                         2 * _lm_n_dofs,
                         2 * _lm_n_dofs + _p_n_dofs);
}

void
NavierStokesHDGKernel::onInternalSide()
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
