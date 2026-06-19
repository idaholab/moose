//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorCorrector.h"
#include "MooseError.h"
#include "NonlinearSystemBase.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/int_range.h"

registerMooseObject("MooseApp", FunctorCorrector);

InputParameters
FunctorCorrector::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Set (all or some) values of (one or more) nonlinear variable(s) using functor evaluations");
  params.addRequiredCoupledVar("variables_to_correct", "Variables to change the value of.");
  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "functors", "Functors to compute the new variable values with");
  params.registerBase("Corrector");
  return params;
}

FunctorCorrector::FunctorCorrector(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    NonADFunctorInterface(this),
    _mesh(_fe_problem.mesh()),
    _var_names(getParam<std::vector<VariableName>>("variables_to_correct")),
    _functor_names(getParam<std::vector<MooseFunctorName>>("functors")),
    _functor_evaluation_technique(getParam<MooseEnum>("functor_evaluation_technique"))
{
  for (const auto & var_name : _var_names)
  {
    auto & var = _fe_problem.getVariable(0, var_name);

    if (_sys.number() != var.sys().system().number())
      paramError("variables_to_correct", "Variables must be all in the non-linear system.");

    _var_numbers.push_back(_sys.system().variable_number(var_name));
  }

  if (_functor_names.size() != _var_names.size())
    paramError("functors",
               "Functor list size (" + std::to_string(_functor_names.size()) +
                   ") should be the same as variable list size (" +
                   std::to_string(_var_names.size()));

  // Retrieve a pointer to the functors
  _functors.resize(_functor_names.size());
  for (const auto i : index_range(_functor_names))
    _functors[i] = &getFunctor<Real>(_functor_names[i], 0);
}

void
FunctorCorrector::initialize()
{
  // do one solution.close to get updated
  _sys.system().solution->close();
}

void
FunctorCorrector::execute()
{
  auto & dof_map = _sys.system().get_dof_map();
  const auto local_dof_begin = dof_map.first_dof();
  const auto local_dof_end = dof_map.end_dof();

  std::vector<std::vector<dof_id_type>> dof_indices(_var_numbers.size());
  std::vector<Real> functor_values(_var_numbers.size());
  const auto state_arg = Moose::currentState();

  if (_functor_evaluation_technique != "node_arg")
    paramError("Only nodal evaluation of functors has been implemented at this time");

  // TODO get desired range
  for (const auto & node : *_mesh.getLocalNodeRange())
  {
    // prepare variable dofs
    for (const auto i : index_range(_var_numbers))
    {
      dof_map.dof_indices(node, dof_indices[i], _var_numbers[i]);
      mooseAssert(dof_indices[i].size() <= 2, "Corrector has not been implemented for this variable type");
    }

    // only doing current for now
    auto & solution = _sys.solutionState(0);

    // loop over all DOFs
    for (const auto j : index_range(dof_indices[0]))
    {
      // check if the first variable's DOFs are local (if they are all other variables should
      // have local DOFS as well)
      if (dof_indices[0][j] > local_dof_end || dof_indices[0][j] < local_dof_begin)
        continue;

      // evaluate functors
      const Moose::NodeArg node_arg = {node,
                                       &Moose::NodeArg::undefined_subdomain_connection};
      for (const auto i : index_range(_var_numbers))
        functor_values[i] = (*_functors[i])(node_arg, state_arg);

      // renormalize
      for (const auto i : index_range(_var_numbers))
        solution.set(dof_indices[i][j], functor_values[i]);
    }
  }
}

void
FunctorCorrector::finalize()
{
  _sys.system().solution->close();
  _sys.update();
}
