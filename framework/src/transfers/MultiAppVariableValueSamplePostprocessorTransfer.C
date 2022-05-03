//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppVariableValueSamplePostprocessorTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MultiApp.h"
#include "AuxiliarySystem.h"
#include "MooseUtils.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

registerMooseObject("MooseApp", MultiAppVariableValueSamplePostprocessorTransfer);

InputParameters
MultiAppVariableValueSamplePostprocessorTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription(
      "Samples the value of a variable within the main application at each sub-application "
      "position and transfers the value to a postprocessor on the sub-application(s) when "
      "performing the to-multiapp transfer. Reconstructs the value of a CONSTANT MONOMIAL "
      "variable associating the value of each element to the value of the postprocessor "
      "in the closest sub-application whem performing the from-multiapp transfer.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor",
      "The name of the postprocessor in the MultiApp to transfer the value to.  "
      "This should most likely be a Reciever Postprocessor.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<unsigned int>("source_variable_component", 0, "The component of source variable");
  return params;
}

MultiAppVariableValueSamplePostprocessorTransfer::MultiAppVariableValueSamplePostprocessorTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _postprocessor_name(getParam<PostprocessorName>("postprocessor")),
    _var_name(getParam<VariableName>("source_variable")),
    _comp(getParam<unsigned int>("source_variable_component")),
    _var(_fe_problem.getVariable(0, _var_name))
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");

  if (isParamValid("from_multi_app"))
  {
    // Check that the variable is a CONSTANT MONOMIAL.
    auto & fe_type = _var.feType();
    if (fe_type.order != CONSTANT || fe_type.family != MONOMIAL)
      paramError("source_variable",
                 "Variable must be in CONSTANT MONOMIAL when transferring from a postprocessor "
                 "from sub-apps.");

    if (!_fe_problem.getAuxiliarySystem().hasVariable(_var_name))
      paramError("source_variable", "Variable must be an auxiliary variable");
  }
}

void
MultiAppVariableValueSamplePostprocessorTransfer::initialSetup()
{
  if (isParamValid("to_multi_app"))
    return;

  // Cache the Multiapp position ID for every element.
  auto & mesh = _fe_problem.mesh().getMesh();
  unsigned int multiapp_pos_id = 0;
  for (auto & elem : as_range(mesh.active_local_elements_begin(), mesh.active_local_elements_end()))
  {
    // Exclude the elements without dofs.
    if (_var.hasBlocks(elem->subdomain_id()))
    {
      Real distance = std::numeric_limits<Real>::max();
      unsigned int count = 0;
      for (unsigned int j = 0; j < getFromMultiApp()->numGlobalApps(); ++j)
      {
        Real current_distance = (getFromMultiApp()->position(j) - elem->true_centroid()).norm();
        if (MooseUtils::absoluteFuzzyLessThan(current_distance, distance))
        {
          distance = current_distance;
          multiapp_pos_id = j;
          count = 0;
        }
        else if (MooseUtils::absoluteFuzzyEqual(current_distance, distance))
          ++count;
      }
      if (count > 0)
        mooseError("The distances of an element to more than one sub-applications are too close."
                   "\nDifferent positions for sub-applications or a centroid-based MultiApp can "
                   "be used to resolve this error.");
      _cached_multiapp_pos_ids.push_back(multiapp_pos_id);
    }
  }
}

void
MultiAppVariableValueSamplePostprocessorTransfer::execute()
{
  TIME_SECTION("MultiAppVariableValueSamplePostprocessorTransfer::execute()",
               5,
               "Transferring a variable to a postprocessor through sampling");

  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      const ArrayMooseVariable * array_var = nullptr;
      const MooseVariableField<Real> * standard_var = nullptr;
      if (_var.isArray())
        array_var = &_fe_problem.getArrayVariable(0, _var_name);
      else if (!_var.isVector())
        standard_var = static_cast<MooseVariableField<Real> *>(&_var);
      else
        mooseError("MultiAppVariableValueSamplePostprocessorTransfer does not support transfer of "
                   "vector variables");

      MooseMesh & from_mesh = _fe_problem.mesh();

      std::unique_ptr<PointLocatorBase> pl = from_mesh.getPointLocator();

      pl->enable_out_of_mesh_mode();

      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
      {
        Real value = -std::numeric_limits<Real>::max();

        { // Get the value of the variable at the point where this multiapp is in the master domain

          Point multi_app_position = getToMultiApp()->position(i);

          std::vector<Point> point_vec(1, multi_app_position);

          // First find the element the hit lands in
          const Elem * elem = (*pl)(multi_app_position);

          if (elem && elem->processor_id() == from_mesh.processor_id())
          {
            _fe_problem.setCurrentSubdomainID(elem, 0);
            _fe_problem.reinitElemPhys(elem, point_vec, 0);

            if (array_var)
            {
              value = array_var->sln()[0](_comp);
              mooseAssert(
                  _comp < array_var->count(),
                  "Component must be smaller than the number of components of array variable!");
              mooseAssert(array_var->sln().size() == 1, "No values in u!");
            }
            else
            {
              value = standard_var->sln()[0];
              mooseAssert(standard_var->sln().size() == 1, "No values in u!");
            }
          }

          _communicator.max(value);
        }

        if (getToMultiApp()->hasLocalApp(i))
          getToMultiApp()->appProblemBase(i).setPostprocessorValueByName(_postprocessor_name,
                                                                         value);
      }

      break;
    }
    case FROM_MULTIAPP:
    {
      auto & mesh = _fe_problem.mesh().getMesh();
      auto & solution = _var.sys().solution();

      // Store the local multiapps postprocessor values.
      const unsigned int n_subapps = getFromMultiApp()->numGlobalApps();
      std::vector<Real> pp_values(n_subapps, std::numeric_limits<Real>::max());
#ifdef DEBUG
      std::vector<Real> duplicate(n_subapps, -std::numeric_limits<Real>::max());
#endif
      for (const auto i : make_range(n_subapps))
        if (getFromMultiApp()->hasLocalApp(i))
        {
          pp_values[i] = getFromMultiApp()->appPostprocessorValue(i, _postprocessor_name);
#ifdef DEBUG
          duplicate[i] = pp_values[i];
#endif
        }

      // Gather all the multiapps postprocessor values.
      _communicator.min(pp_values);
#ifdef DEBUG
      _communicator.max(duplicate);
      for (const auto i : make_range(n_subapps))
        if (pp_values[i] != duplicate[i])
          mooseError(
              "There should be only one processor setting a subapp postprocessor but now this "
              "appears not true.");
#endif

      // Assign the multiapps postprocessor values to the local elements.
      unsigned int i = 0;
      for (auto & elem :
           as_range(mesh.active_local_elements_begin(), mesh.active_local_elements_end()))
      {
        // Exclude the elements without dofs
        if (_var.hasBlocks(elem->subdomain_id()))
        {
          std::vector<dof_id_type> dof_indices;
          _var.getDofIndices(elem, dof_indices);
          mooseAssert(dof_indices.size() == 1,
                      "The variable must be in constant monomial with one DoF on an element");
          solution.set(dof_indices[0], pp_values[_cached_multiapp_pos_ids[i]]);
          ++i;
        }
      }
      solution.close();
      break;
    }
  }
}
