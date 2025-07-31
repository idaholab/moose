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

#include "libmesh/dof_map.h"

registerMooseObject("MooseApp", DiscreteVariableResidualNorm);

InputParameters
DiscreteVariableResidualNorm::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();

  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable to compute the residual for");
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
  : ElementPostprocessor(parameters),
    _var(_fe_problem.getVariable(_tid,
                                 getParam<VariableName>("variable"),
                                 Moose::VarKindType::VAR_SOLVER,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)),
    _norm_type(getParam<MooseEnum>("norm_type").getEnum<NormType>()),
    _correct_mesh_bias(getParam<bool>("correct_mesh_bias")),
    _nl_residual_vector(_fe_problem.getNonlinearSystemBase(_sys.number()).RHS())
{
}

void
DiscreteVariableResidualNorm::initialize()
{
  _norm = 0;
  _local_dof_indices.clear();
  _nonlocal_dof_indices_map.clear();
}

void
DiscreteVariableResidualNorm::execute()
{
  for (const auto dof_index : _var.dofIndices())
  {
    // Dof indices may not be owned by the same processor as the current element
    if (_var.dofMap().local_index(dof_index))
      _local_dof_indices.insert(dof_index);
    else
    {
      // if a Dof is non-local add it to a map, to be communicated to the owner in finalize()
      const auto dof_owner = _var.dofMap().dof_owner(dof_index);
      _nonlocal_dof_indices_map[dof_owner].push_back(dof_index);
    }
  }
}

void
DiscreteVariableResidualNorm::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const DiscreteVariableResidualNorm &>(y);
  _local_dof_indices.insert(pps._local_dof_indices.begin(), pps._local_dof_indices.end());
}

void
DiscreteVariableResidualNorm::finalize()
{
  // communicate the non-local Dofs to their processors
  auto receive_functor = [&](processor_id_type /*pid*/, const std::vector<dof_id_type> & indices)
  { _local_dof_indices.insert(indices.begin(), indices.end()); };
  Parallel::push_parallel_vector_data(_communicator, _nonlocal_dof_indices_map, receive_functor);

  // compute the total number of Dofs for the variable on the subdomain
  unsigned int n_dofs = _local_dof_indices.size();
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
}

PostprocessorValue
DiscreteVariableResidualNorm::getValue() const
{
  return _norm;
}
