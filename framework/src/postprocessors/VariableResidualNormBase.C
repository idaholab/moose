//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableResidualNormBase.h"
#include "MooseVariableFieldBase.h"
#include "NonlinearSystemBase.h"

#include "libmesh/dof_map.h"

InputParameters
VariableResidualNormBase::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();

  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable to compute the residual for");
  params.addParam<bool>("include_scaling_factor",
                        false,
                        "If set to true, include the residual scaling factor in the norm; "
                        "otherwise, divide by the scaling factor");

  return params;
}

VariableResidualNormBase::VariableResidualNormBase(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _var(_fe_problem.getVariable(_tid,
                                 getParam<VariableName>("variable"),
                                 Moose::VarKindType::VAR_SOLVER,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)),
    _include_scaling_factor(getParam<bool>("include_scaling_factor")),
    _nl_residual_vector(_fe_problem.getNonlinearSystemBase(_sys.number()).RHS())
{
}

void
VariableResidualNormBase::initialize()
{
  _norm = 0;
  _local_dof_indices.clear();
  _nonlocal_dof_indices_map.clear();
}

void
VariableResidualNormBase::execute()
{
  const auto dof_indices = getCurrentElemDofIndices();

  for (const auto dof_index : dof_indices)
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
VariableResidualNormBase::threadJoin(const UserObject & y)
{
  const auto & pps = static_cast<const VariableResidualNormBase &>(y);
  _local_dof_indices.insert(pps._local_dof_indices.begin(), pps._local_dof_indices.end());
}

void
VariableResidualNormBase::finalize()
{
  // communicate the non-local Dofs to their processors
  auto receive_functor = [&](processor_id_type /*pid*/, const std::vector<dof_id_type> & indices)
  { _local_dof_indices.insert(indices.begin(), indices.end()); };
  Parallel::push_parallel_vector_data(_communicator, _nonlocal_dof_indices_map, receive_functor);

  computeNorm();
}

PostprocessorValue
VariableResidualNormBase::getValue() const
{
  return _norm;
}
