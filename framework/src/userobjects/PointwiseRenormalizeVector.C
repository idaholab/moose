//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointwiseRenormalizeVector.h"
#include "MooseError.h"
#include "NonlinearSystemBase.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/int_range.h"

registerMooseObject("MooseApp", PointwiseRenormalizeVector);

InputParameters
PointwiseRenormalizeVector::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Pointwise renormalize the solution of a set of variables comprising a vector");
  params.addCoupledVar("v", "Variables comprising the vector");
  params.addParam<Real>("norm", 1.0, "Desired norm for the coupled variable vector");
  return params;
}

PointwiseRenormalizeVector::PointwiseRenormalizeVector(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_fe_problem.mesh()),
    _var_names(getParam<std::vector<VariableName>>("v")),
    _target_norm(getParam<Real>("norm")),
    _nl_sys(_fe_problem.getNonlinearSystemBase()),
    _sys(_nl_sys.system())
{
  const MooseVariableFieldBase * first_var = nullptr;
  for (const auto & var_name : _var_names)
  {
    auto & var = _fe_problem.getVariable(0, var_name);
    if (!first_var)
      first_var = &var;
    else
    {
      // check order and family for consistency
      if (first_var->feType() != var.feType())
        paramError("v", "All supplied variables must be of the same order and family.");
      // check block restriction for consistency
      if (!first_var->hasBlocks(var.blockIDs()) || !var.hasBlocks(first_var->blockIDs()))
        paramError("v", "All supplied variables must have the same block restriction.");
    }

    if (_sys.number() != var.sys().system().number())
      paramError("v", "Variables must be all in the non-linear system.");

    if (var.isArray())
    {
      const auto & array_var = _fe_problem.getArrayVariable(0, var_name);
      for (unsigned int p = 0; p < var.count(); ++p)
        _var_numbers.push_back(_sys.variable_number(array_var.componentName(p)));
    }
    else
      _var_numbers.push_back(_sys.variable_number(var_name));
  }
}

void
PointwiseRenormalizeVector::initialize()
{
  // do one solution.close to get updated
  _sys.solution->close();
}

void
PointwiseRenormalizeVector::execute()
{
  auto & dof_map = _sys.get_dof_map();
  const auto local_dof_begin = dof_map.first_dof();
  const auto local_dof_end = dof_map.end_dof();

  std::vector<std::vector<dof_id_type>> dof_indices(_var_numbers.size());
  std::vector<Real> cache(_var_numbers.size());

  for (const auto & elem : *_mesh.getActiveLocalElementRange())
  {
    // prepare variable dofs
    for (const auto i : index_range(_var_numbers))
    {
      dof_map.dof_indices(elem, dof_indices[i], _var_numbers[i]);

      // check that all vars have the same number of dofs
      mooseAssert(dof_indices[i].size() == dof_indices[0].size(),
                  "All specified variables should have the same number of DOFs");
    }

    // iterate over current, old, and older solutions
    for (const auto s : make_range(3))
      if (_nl_sys.hasSolutionState(s))
      {
        auto & solution = _nl_sys.solutionState(s);

        // loop over all DOFs
        for (const auto j : index_range(dof_indices[0]))
        {
          // check if the first variable's DOFs are local (if they are all other variables should
          // have local DOFS as well)
          if (dof_indices[0][j] > local_dof_end || dof_indices[0][j] < local_dof_begin)
            continue;

          // compute current norm
          Real norm = 0.0;
          for (const auto i : index_range(_var_numbers))
            norm += Utility::pow<2>(solution(dof_indices[i][j]));

          if (norm == 0.0)
            continue;
          norm = std::sqrt(norm);

          // renormalize
          for (const auto i : index_range(_var_numbers))
            solution.set(dof_indices[i][j], solution(dof_indices[i][j]) / norm * _target_norm);
        }
      }
  }
}

void
PointwiseRenormalizeVector::finalize()
{
  for (const auto s : make_range(3))
    if (_nl_sys.hasSolutionState(s))
      _nl_sys.solutionState(s).close();

  _sys.update();
}
