//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteVariableResidualNorm.h"
#include "MooseVariableFieldBase.h"
#include "NonlinearSystemBase.h"

registerMooseObject("MooseApp", DiscreteVariableResidualNorm);

InputParameters
DiscreteVariableResidualNorm::validParams()
{
  InputParameters params = VariableResidualNormBase::validParams();

  MooseEnum norm_type("l_1=0 l_2=1 l_inf=2");
  norm_type.addDocumentation("l_1", "l-1 norm");
  norm_type.addDocumentation("l_2", "l-2 norm");
  norm_type.addDocumentation("l_inf", "l-infinity norm");
  params.addRequiredParam<MooseEnum>("norm_type", norm_type, "Type of discrete norm to compute");
  params.addParam<bool>(
      "correct_mesh_bias",
      false,
      "If set to true, correct the mesh size bias associated with the selected norm. For l-1, "
      "divide by N, the number of block-restricted DoFs for the variable. For l-2, divide by "
      "sqrt(N). For l-infinity, no correction needs to be made.");

  params.addClassDescription("Computes a discrete norm for a block-restricted variable residual.");

  return params;
}

DiscreteVariableResidualNorm::DiscreteVariableResidualNorm(const InputParameters & parameters)
  : VariableResidualNormBase(parameters),
    _norm_type(getParam<MooseEnum>("norm_type").getEnum<NormType>()),
    _correct_mesh_bias(getParam<bool>("correct_mesh_bias"))
{
}

std::vector<dof_id_type>
DiscreteVariableResidualNorm::getCurrentElemDofIndices() const
{
  return _var.dofIndices();
}

void
DiscreteVariableResidualNorm::computeNorm()
{
  // compute the total number of Dofs for the variable on the subdomain
  auto n_dofs = _local_dof_indices.size();
  gatherSum(n_dofs);

  Real bias = 1.0;
  switch (_norm_type)
  {
    case NormType::l_1:
      _norm = _nl_residual_vector.subset_l1_norm(_local_dof_indices);
      bias = n_dofs;
      break;
    case NormType::l_2:
      _norm = _nl_residual_vector.subset_l2_norm(_local_dof_indices);
      bias = sqrt(n_dofs);
      break;
    case NormType::l_inf:
      _norm = _nl_residual_vector.subset_linfty_norm(_local_dof_indices);
      break;
    default:
      mooseError("Invalid norm type");
  }

  if (_correct_mesh_bias)
    _norm /= bias;

  if (!_include_scaling_factor)
    _norm /= _var.scalingFactor();
}
