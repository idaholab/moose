//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHybridizedKernel.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"

registerMooseObject("MooseApp", DiffusionHybridizedKernel);

InputParameters
DiffusionHybridizedKernel::validParams()
{
  auto params = HybridizedKernel::validParams();
  params += DiffusionHybridizedInterface::validParams();
  params.addParam<FunctionName>("source", 0, "Source for the diffusing species");
  params.addClassDescription("Implements the diffusion equation for a hybridized discretization");
  return params;
}

DiffusionHybridizedKernel::DiffusionHybridizedKernel(const InputParameters & parameters)
  : HybridizedKernel(parameters),
    DiffusionHybridizedInterface(this, this, _sys, _aux_sys, _tid),
    _source(getFunction("source"))
{
}

void
DiffusionHybridizedKernel::onElement()
{
  resizeData(*this);

  // Populate LM dof indices
  _lm_dof_indices = _lm_u_dof_indices;

  // Populate primal dof indices if we are computing the primal increment
  if (!computingGlobalData())
  {
    _primal_dof_indices = _qu_dof_indices;
    _primal_dof_indices.insert(
        _primal_dof_indices.end(), _u_dof_indices.begin(), _u_dof_indices.end());
  }

  // qu and u
  vectorVolumeResidual(0, _qu_sol, _u_sol);
  scalarVolumeResidual(_vector_n_dofs, _qu_sol);
  vectorVolumeJacobian(0, 0, _vector_n_dofs);
  scalarVolumeJacobian(_vector_n_dofs, 0);
}

void
DiffusionHybridizedKernel::onInternalSide()
{
  // qu, u, lm_u
  vectorFaceResidual(*this, 0, _lm_u_sol);
  vectorFaceJacobian(*this, 0, 0);
  scalarFaceResidual(*this, _vector_n_dofs, _qu_sol, _u_sol, _lm_u_sol);
  scalarFaceJacobian(*this, _vector_n_dofs, 0, _vector_n_dofs, 0);
  lmFaceResidual(*this, 0, _qu_sol, _u_sol, _lm_u_sol);
  lmFaceJacobian(*this, 0, 0, _vector_n_dofs, 0);
}

void
DiffusionHybridizedKernel::vectorVolumeResidual(const unsigned int i_offset,
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
DiffusionHybridizedKernel::vectorVolumeJacobian(const unsigned int i_offset,
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
DiffusionHybridizedKernel::scalarVolumeResidual(const unsigned int i_offset,
                                                const MooseArray<Gradient> & vector_field)
{
  for (const auto qp : make_range(_qrule->n_points()))
  {
    // Evaluate body force
    const auto f = _source.value(_t, _q_point[qp]);

    for (const auto i : make_range(_scalar_n_dofs))
    {
      _PrimalVec(i_offset + i) +=
          _JxW[qp] * (_grad_scalar_phi[i][qp] * _diff[qp] * vector_field[qp]);

      // Scalar equation RHS
      _PrimalVec(i_offset + i) -= _JxW[qp] * _scalar_phi[i][qp] * f;
    }
  }
}

void
DiffusionHybridizedKernel::scalarVolumeJacobian(const unsigned int i_offset,
                                                const unsigned int vector_field_j_offset)
{
  for (const auto qp : make_range(_qrule->n_points()))
    for (const auto i : make_range(_scalar_n_dofs))
      // Scalar equation dependence on vector dofs
      for (const auto j : make_range(_vector_n_dofs))
        _PrimalMat(i_offset + i, vector_field_j_offset + j) +=
            _JxW[qp] * _diff[qp] * (_grad_scalar_phi[i][qp] * _vector_phi[j][qp]);
}
