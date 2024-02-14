//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHybridizedDirichletBC.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "DiffusionHybridizedKernel.h"

registerMooseObject("MooseApp", DiffusionHybridizedDirichletBC);

InputParameters
DiffusionHybridizedDirichletBC::validParams()
{
  auto params = HybridizedIntegratedBC::validParams();
  params += DiffusionHybridizedInterface::validParams();
  params.addClassDescription("Weakly imposes Dirichlet boundary conditions for a "
                             "hybridized discretization of diffusion equation");
  params.addParam<FunctionName>("function", 0, "The Dirichlet value for the diffusing specie");
  return params;
}

DiffusionHybridizedDirichletBC::DiffusionHybridizedDirichletBC(const InputParameters & parameters)
  : HybridizedIntegratedBC(parameters),
    DiffusionHybridizedInterface(this, this, _sys, _aux_sys, _tid),
    _dirichlet_val(getFunction("function"))
{
}

void
DiffusionHybridizedDirichletBC::onBoundary()
{
  resizeData(*this);

  // qu, u
  vectorDirichletResidual(0);
  scalarDirichletResidual(_vector_n_dofs, _qu_sol, _u_sol);
  scalarDirichletJacobian(_vector_n_dofs, 0, _vector_n_dofs);

  // Set the LMs on these Dirichlet boundary faces to 0
  createIdentityResidual(_lm_phi_face, _lm_u_sol, _lm_n_dofs, 0);
  createIdentityJacobian(_lm_phi_face, _lm_n_dofs, 0);
}

void
DiffusionHybridizedDirichletBC::vectorDirichletResidual(const unsigned int i_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    const auto scalar_value = _dirichlet_val.value(_t, _q_point[qp]);

    // External boundary -> Dirichlet faces -> Vector equation RHS
    for (const auto i : make_range(_vector_n_dofs))
      _PrimalVec(i_offset + i) -=
          _JxW[qp] * (_vector_phi_face[i][qp] * _normals[qp]) * scalar_value;
  }
}

void
DiffusionHybridizedDirichletBC::scalarDirichletResidual(const unsigned int i_offset,
                                                        const MooseArray<Gradient> & vector_sol,
                                                        const MooseArray<Number> & scalar_sol)
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    const auto scalar_value = _dirichlet_val.value(_t, _q_point[qp]);

    for (const auto i : make_range(_scalar_n_dofs))
    {
      // vector
      _PrimalVec(i_offset + i) -=
          _JxW[qp] * _diff[qp] * _scalar_phi_face[i][qp] * (vector_sol[qp] * _normals[qp]);

      // scalar from stabilization term
      _PrimalVec(i_offset + i) +=
          _JxW[qp] * _scalar_phi_face[i][qp] * _tau * scalar_sol[qp] * _normals[qp] * _normals[qp];

      // dirichlet lm from stabilization term
      _PrimalVec(i_offset + i) -=
          _JxW[qp] * _scalar_phi_face[i][qp] * _tau * scalar_value * _normals[qp] * _normals[qp];
    }
  }
}

void
DiffusionHybridizedDirichletBC::scalarDirichletJacobian(const unsigned int i_offset,
                                                        const unsigned int vector_j_offset,
                                                        const unsigned int scalar_j_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : make_range(_scalar_n_dofs))
    {
      for (const auto j : make_range(_vector_n_dofs))
        _PrimalMat(i_offset + i, vector_j_offset + j) -= _JxW[qp] * _diff[qp] *
                                                         _scalar_phi_face[i][qp] *
                                                         (_vector_phi_face[j][qp] * _normals[qp]);

      for (const auto j : make_range(_scalar_n_dofs))
        _PrimalMat(i_offset + i, scalar_j_offset + j) += _JxW[qp] * _scalar_phi_face[i][qp] * _tau *
                                                         _scalar_phi_face[j][qp] * _normals[qp] *
                                                         _normals[qp];
    }
}
