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
  _dirichlet_vel.push_back(&getFunction("dirichlet_u"));
  _dirichlet_vel.push_back(&getFunction("dirichlet_v"));
  _dirichlet_vel.push_back(&getFunction("dirichlet_w"));
}

void
NavierStokesHybridizedVelocityDirichletBC::assemble()
{
  resizeData(*this);

  // qu, u, lm_u
  vectorDirichletResidual(0, 0);
  scalarDirichletResidual(_vector_n_dofs, _qu_sol, _u_sol, 0);
  scalarDirichletJacobian(_vector_n_dofs, 0, _vector_n_dofs, 2 * _lm_n_dofs, 0);

  // qv, v, lm_v
  vectorDirichletResidual(_vector_n_dofs + _scalar_n_dofs, 1);
  scalarDirichletResidual(2 * _vector_n_dofs + _scalar_n_dofs, _qv_sol, _v_sol, 1);
  scalarDirichletJacobian(2 * _vector_n_dofs + _scalar_n_dofs,
                          _vector_n_dofs + _scalar_n_dofs,
                          2 * _vector_n_dofs + _scalar_n_dofs,
                          2 * _lm_n_dofs,
                          1);

  // p
  pressureDirichletResidual(2 * _lm_n_dofs);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityResidual(_lm_phi_face, _lm_u_sol, _lm_n_dofs, 0);
  createIdentityResidual(_lm_phi_face, _lm_v_sol, _lm_n_dofs, _lm_n_dofs);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, 0);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, _lm_n_dofs);
}

RealVectorValue
NavierStokesHybridizedVelocityDirichletBC::getDirichletVelocity(const unsigned int qp) const
{
  return RealVectorValue(_dirichlet_vel[0]->value(_t, _q_point[qp]),
                         _dirichlet_vel[1]->value(_t, _q_point[qp]),
                         _dirichlet_vel[2]->value(_t, _q_point[qp]));
}

void
NavierStokesHybridizedVelocityDirichletBC::pressureDirichletResidual(const unsigned int i_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    const auto dirichlet_velocity = getDirichletVelocity(qp);
    const auto vdotn = dirichlet_velocity * _normals[qp];
    for (const auto i : make_range(_p_n_dofs))
      _LMVec(i_offset + i) += _JxW[qp] * vdotn * _scalar_phi_face[i][qp];
  }
}

void
NavierStokesHybridizedVelocityDirichletBC::vectorDirichletResidual(const unsigned int i_offset,
                                                                   const unsigned int vel_component)
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    const auto scalar_value = getDirichletVelocity(qp)(vel_component);

    // External boundary -> Dirichlet faces -> Vector equation RHS
    for (const auto i : make_range(_vector_n_dofs))
      _PrimalVec(i_offset + i) -=
          _JxW[qp] * (_vector_phi_face[i][qp] * _normals[qp]) * scalar_value;
  }
}

void
NavierStokesHybridizedVelocityDirichletBC::scalarDirichletResidual(
    const unsigned int i_offset,
    const MooseArray<Gradient> & vector_sol,
    const MooseArray<Number> & scalar_sol,
    const unsigned int vel_component)
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];

    const auto dirichlet_velocity = getDirichletVelocity(qp);
    const auto scalar_value = dirichlet_velocity(vel_component);
    ;

    for (const auto i : make_range(_scalar_n_dofs))
    {
      // vector
      _PrimalVec(i_offset + i) -=
          _JxW[qp] * _nu[qp] * _scalar_phi_face[i][qp] * (vector_sol[qp] * _normals[qp]);

      // pressure
      _PrimalVec(i_offset + i) += _JxW[qp] * _scalar_phi_face[i][qp] * (qp_p * _normals[qp]);

      // scalar from stabilization term
      _PrimalVec(i_offset + i) +=
          _JxW[qp] * _scalar_phi_face[i][qp] * _tau * scalar_sol[qp] * _normals[qp] * _normals[qp];

      // dirichlet lm from stabilization term
      _PrimalVec(i_offset + i) -=
          _JxW[qp] * _scalar_phi_face[i][qp] * _tau * scalar_value * _normals[qp] * _normals[qp];

      // dirichlet lm from advection term
      _PrimalVec(i_offset + i) +=
          _JxW[qp] * _scalar_phi_face[i][qp] * (dirichlet_velocity * _normals[qp]) * scalar_value;
    }
  }
}

void
NavierStokesHybridizedVelocityDirichletBC::scalarDirichletJacobian(
    const unsigned int i_offset,
    const unsigned int vector_j_offset,
    const unsigned int scalar_j_offset,
    const unsigned int p_j_offset,
    const unsigned int vel_component)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : make_range(_scalar_n_dofs))
    {
      for (const auto j : make_range(_vector_n_dofs))
        _PrimalMat(i_offset + i, vector_j_offset + j) -=
            _JxW[qp] * _nu[qp] * _scalar_phi_face[i][qp] * (_vector_phi_face[j][qp] * _normals[qp]);

      for (const auto j : make_range(_p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi_face[j][qp];
        // pressure
        _PrimalLM(i_offset + i, p_j_offset + j) +=
            _JxW[qp] * _scalar_phi_face[i][qp] * (p_phi * _normals[qp]);
      }

      for (const auto j : make_range(_scalar_n_dofs))
        _PrimalMat(i_offset + i, scalar_j_offset + j) += _JxW[qp] * _scalar_phi_face[i][qp] * _tau *
                                                         _scalar_phi_face[j][qp] * _normals[qp] *
                                                         _normals[qp];
    }
}
