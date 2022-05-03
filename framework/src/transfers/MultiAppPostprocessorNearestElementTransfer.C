//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppPostprocessorNearestElementTransfer.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MultiApp.h"
#include "SystemBase.h"

registerMooseObject("MooseApp", MultiAppPostprocessorNearestElementTransfer);

InputParameters
MultiAppPostprocessorNearestElementTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription(
      "Reconstruct the value of a CONSTANT MONOMIAL variable associating the "
      "value of each element to the value of the postprocessor in the closest "
      "MultiApp.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the postprocessor in the MultiApp to transfer the value from.");
  params.addRequiredParam<VariableName>("source_variable",
                                        "The CONSTANT "
                                        "MONOMIAL variable to transfer to.");
  params.addParam<unsigned int>("source_variable_component", 0, "The component of source variable");
  return params;
}

MultiAppPostprocessorNearestElementTransfer::MultiAppPostprocessorNearestElementTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _postprocessor_name(getParam<PostprocessorName>("postprocessor")),
    _to_var_name(getParam<VariableName>("source_variable")),
    _to_problem(getFromMultiApp()->problemBase()),
    _to_var(_to_problem.getVariable(0, _to_var_name)),
    _to_sys(_to_var.sys().system()),
    _to_mesh(_to_problem.mesh().getMesh()),
    _to_sys_num(_to_sys.number()),
    _to_var_num(_to_sys.variable_number(
        _to_var.componentName(getParam<unsigned int>("source_variable_component"))))
{
  // Check the number of transfer directions.
  if (_directions.size() != 1 && _directions.get(0) != FROM_MULTIAPP)
    mooseError("MultiAppPostprocessorNearestElementTransfer works only with direction equal to "
               "FROM_MULTIAPP");

  // Check that the variable is a CONSTANT MONOMIAL.
  auto & to_fe_type = _to_sys.variable_type(_to_var_num);
  if (to_fe_type.order != CONSTANT || to_fe_type.family != MONOMIAL)
    mooseError(parameters.blockLocation() + " " + parameters.blockFullpath() +
               "\nMultiAppPostprocessorNearestElementTransfer works with CONSTANT MONOMIAL "
               "variables only");
}

void
MultiAppPostprocessorNearestElementTransfer::initialSetup()
{
  // Cache the Multiapp position ID for every element.
  unsigned int multiapp_pos_id = 0;
  for (auto & elem :
       as_range(_to_mesh.active_local_elements_begin(), _to_mesh.active_local_elements_end()))
  {
    // Exclude the elements without dofs.
    if (elem->n_dofs(_to_sys_num, _to_var_num) > 0)
    {
      Real distance = std::numeric_limits<Real>::max();
      for (unsigned int j = 0; j < getFromMultiApp()->numGlobalApps(); ++j)
      {
        Real current_distance = (getFromMultiApp()->position(j) - elem->true_centroid()).norm();
        if (current_distance < distance)
        {
          distance = current_distance;
          multiapp_pos_id = j;
        }
      }
      _cached_multiapp_pos_ids.push_back(multiapp_pos_id);
    }
  }
}

void
MultiAppPostprocessorNearestElementTransfer::execute()
{
  // Check that transfer direction is FROM_MULTIAPP.
  if (_current_direction != FROM_MULTIAPP)
    paramError("direction", "This transfer works only with direction equal to FROM_MULTIAPP");

  // Retrieve the vector of the variable values.
  NumericVector<Real> & solution = *_to_sys.solution;

  // Store the local multiapps postprocessor values.
  const unsigned int n_subapps = getFromMultiApp()->numGlobalApps();
  std::vector<Real> pp_values(n_subapps, std::numeric_limits<Real>::max());
  std::vector<Real> duplicate(n_subapps, -std::numeric_limits<Real>::max());
  for (const auto i : make_range(n_subapps))
    if (getFromMultiApp()->hasLocalApp(i))
    {
      pp_values[i] = getFromMultiApp()->appPostprocessorValue(i, _postprocessor_name);
      duplicate[i] = pp_values[i];
    }

  // Gather all the multiapps postprocessor values.
  _communicator.min(pp_values);
  _communicator.max(duplicate);
  for (const auto i : make_range(n_subapps))
    if (pp_values[i] != duplicate[i])
      mooseError("There should be only one processor setting a subapp postprocessor but now this "
                 "appears not true.");

  // Assign the multiapps postprocessor values to the local elements.
  unsigned int i = 0;
  for (auto & elem :
       as_range(_to_mesh.active_local_elements_begin(), _to_mesh.active_local_elements_end()))
  {
    // Exclude the elements without dofs
    if (elem->n_dofs(_to_sys_num, _to_var_num) > 0)
    {
      dof_id_type dof = elem->dof_number(_to_sys_num, _to_var_num, 0);
      solution.set(dof, pp_values[_cached_multiapp_pos_ids[i]]);
      ++i;
    }
  }
  solution.close();
}
