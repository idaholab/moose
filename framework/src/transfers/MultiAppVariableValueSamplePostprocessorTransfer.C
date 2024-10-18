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
#include "MooseAppCoordTransform.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

#include "timpi/parallel_sync.h"

using namespace libMesh;

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
  params.addParam<unsigned int>(
      "source_variable_component",
      0,
      "The component of source variable, may be non-zero for array variables.");
  params.addParam<bool>(
      "map_array_variable_components_to_child_apps",
      false,
      "When true, groups of sub-applications will be associated with different components of the "
      "supplied array variable in 'source_variable'. For instance, if there are 9 sub-applications "
      "and 3 components in the variable, sub-apps 0-2 will go to component 0, 3-5 will go to 1, "
      "and 6-8 will go to 2.");
  return params;
}

MultiAppVariableValueSamplePostprocessorTransfer::MultiAppVariableValueSamplePostprocessorTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    MeshChangedInterface(parameters),
    _postprocessor_name(getParam<PostprocessorName>("postprocessor")),
    _var_name(getParam<VariableName>("source_variable")),
    _comp(getParam<unsigned int>("source_variable_component")),
    _var(_fe_problem.getVariable(0, _var_name)),
    _map_comp_to_child(getParam<bool>("map_array_variable_components_to_child_apps"))
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");

  if (_directions.isValueSet("from_multiapp"))
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
  else if (_directions.isValueSet("between_multiapp"))
    mooseError("MultiAppVariableValueSamplePostprocessorTransfer has not been made to support "
               "sibling transfers");

  if (_map_comp_to_child && !_var.isArray())
    paramError("map_array_variable_components_to_child_apps",
               "'source_variable' must be an array variable when mapping array variable components "
               "to child applications.");
  if (_map_comp_to_child && parameters.isParamSetByUser("source_variable_component"))
    paramError("map_array_variable_components_to_child_apps",
               "'source_variable_component' is invalid when mapping array variable components to "
               "child applications.");
}

void
MultiAppVariableValueSamplePostprocessorTransfer::setupPostprocessorCommunication()
{
  if (!_directions.isValueSet("from_multiapp"))
    return;

  const auto num_global_apps = getFromMultiApp()->numGlobalApps();

  // Setup the communication pattern
  _postprocessor_to_processor_id.resize(num_global_apps,
                                        std::numeric_limits<processor_id_type>::max());
  for (const auto i : make_range(num_global_apps))
    if (getFromMultiApp()->hasLocalApp(i))
      _postprocessor_to_processor_id[i] = this->processor_id();

  _communicator.min(_postprocessor_to_processor_id);
#ifdef DEBUG
  for (const auto i : make_range(num_global_apps))
  {
    mooseAssert(_postprocessor_to_processor_id[i] != std::numeric_limits<processor_id_type>::max(),
                "Every element in the vector should have been set.");
    if (getFromMultiApp()->hasLocalApp(i))
      mooseAssert(_postprocessor_to_processor_id[i] == this->processor_id(),
                  "If I owned this app, then the processor id value should be my own");
  }
#endif
}

void
MultiAppVariableValueSamplePostprocessorTransfer::cacheElemToPostprocessorData()
{
  if (!_directions.isValueSet("from_multiapp"))
    return;

  // Cache the Multiapp position ID for every element.
  auto & mesh = _fe_problem.mesh().getMesh();
  unsigned int multiapp_pos_id = 0;
  for (auto & elem : as_range(mesh.active_local_elements_begin(), mesh.active_local_elements_end()))
    // Exclude the elements without dofs.
    if (_var.hasBlocks(elem->subdomain_id()))
    {
      // The next two loops will loop through all the sub-applications
      // The first loop is over each component of the source variable we are transferring to/from
      unsigned int j = 0; // Indicates sub-app index
      for (unsigned int g = 0; g < getFromMultiApp()->numGlobalApps() / _apps_per_component; ++g)
      {
        Real distance = std::numeric_limits<Real>::max();
        unsigned int count = 0;
        // The second loop is over all the sub-apps the given component is associated with
        for (unsigned int c = 0; c < _apps_per_component; ++c, ++j)
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
          mooseWarning(
              "The distances of an element to more than one sub-applications are too close "
              " in transfer '",
              name(),
              "'. The code chooses the sub-application with the smallest ID to set "
              "the variable on the element, which may created undesired variable solutions."
              "\nHaving different positions for sub-applications, "
              "a centroid-based MultiApp or adding block restriction to the variable can "
              "be used to resolve this warning.");

        // Note: in case of count>0, the sub-application with smallest id will be used for the
        //       transfer.
        _cached_multiapp_pos_ids.push_back(multiapp_pos_id);
        _needed_postprocessors.insert(multiapp_pos_id);
      }
    }
}

void
MultiAppVariableValueSamplePostprocessorTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();

  unsigned int num_apps = _directions.isValueSet("from_multiapp")
                              ? getFromMultiApp()->numGlobalApps()
                              : getToMultiApp()->numGlobalApps();
  if (_map_comp_to_child && num_apps % _var.count() != 0)
    paramError("map_array_variable_components_to_child_apps",
               "The number of sub-applications (",
               num_apps,
               ") is not divisible by the number of components in '",
               _var_name,
               "' (",
               _var.count(),
               ").");
  _apps_per_component = _map_comp_to_child ? num_apps / _var.count() : num_apps;

  setupPostprocessorCommunication();
  cacheElemToPostprocessorData();
}

void
MultiAppVariableValueSamplePostprocessorTransfer::meshChanged()
{
  cacheElemToPostprocessorData();
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

      auto active_tags = _fe_problem.getActiveFEVariableCoupleableVectorTags(/*thread_id=*/0);
      std::set<unsigned int> solution_tag = {_fe_problem.getVectorTagID(Moose::SOLUTION_TAG)};

      _fe_problem.setActiveFEVariableCoupleableVectorTags(solution_tag, /*thread_id=*/0);

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
              value = array_var->sln()[0](getVariableComponent(i));
              mooseAssert(
                  getVariableComponent(i) < array_var->count(),
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

      _fe_problem.setActiveFEVariableCoupleableVectorTags(active_tags, /*thread_id=*/0);

      break;
    }
    case FROM_MULTIAPP:
    {
      auto & mesh = _fe_problem.mesh().getMesh();
      auto & solution = _var.sys().solution();

      // Get the required postprocessor values
      const unsigned int n_subapps = getFromMultiApp()->numGlobalApps();
      std::vector<Real> pp_values(n_subapps, std::numeric_limits<Real>::max());
      for (const auto i : make_range(n_subapps))
        if (getFromMultiApp()->hasLocalApp(i))
          pp_values[i] = getFromMultiApp()->appPostprocessorValue(i, _postprocessor_name);

      // Gather all the multiapps postprocessor values that we need
      std::unordered_map<processor_id_type, std::vector<unsigned int>> postprocessor_queries;
      for (const auto needed_postprocessor : _needed_postprocessors)
      {
        const auto proc_id = _postprocessor_to_processor_id[needed_postprocessor];
        if (proc_id != this->processor_id())
          postprocessor_queries[proc_id].push_back(needed_postprocessor);
      }

      auto gather_data = [&pp_values
#ifndef NDEBUG
                          ,
                          this
#endif
      ](processor_id_type libmesh_dbg_var(pid),
                         const std::vector<unsigned int> & postprocessor_ids,
                         std::vector<Real> & postprocessor_values)
      {
        mooseAssert(pid != this->processor_id(), "Should not be pulling from self");
        postprocessor_values.resize(postprocessor_ids.size());
        for (const auto i : index_range(postprocessor_ids))
        {
          const auto pp_id = postprocessor_ids[i];
          const auto pp_value = pp_values[pp_id];
          mooseAssert(
              pp_value != std::numeric_limits<Real>::max(),
              "If we are getting queried for postprocessor data, then we better have a valid"
              "postprocesor value.");
          postprocessor_values[i] = pp_value;
        }
      };

      auto act_on_data = [&pp_values
#ifndef NDEBUG
                          ,
                          this
#endif
      ](processor_id_type libmesh_dbg_var(pid),
                         const std::vector<unsigned int> & postprocessor_ids,
                         const std::vector<Real> & postprocessor_values)
      {
        mooseAssert(pid != this->processor_id(), "Should not be returning a query from self");
        mooseAssert(postprocessor_ids.size() == postprocessor_values.size(),
                    "should be a 1-to-1 query-to-response");
        for (const auto i : index_range(postprocessor_ids))
        {
          const auto pp_id = postprocessor_ids[i];
          const auto pp_value = postprocessor_values[i];
          mooseAssert(pp_value != std::numeric_limits<Real>::max(),
                      "If we are returning postprocessor data, then we better have a valid"
                      "postprocesor value.");
          pp_values[pp_id] = pp_value;
        }
      };

      constexpr Real example = 0;
      TIMPI::pull_parallel_vector_data(
          _communicator, postprocessor_queries, gather_data, act_on_data, &example);

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
                      "The variable must be a constant monomial with one DoF on an element");
          mooseAssert(pp_values[_cached_multiapp_pos_ids[i]] != std::numeric_limits<Real>::max(),
                      "We should have pulled all the data we needed.");
          for (unsigned int c = 0; c < n_subapps / _apps_per_component; ++c)
          {
            solution.set(dof_indices[0] + getVariableComponent(_cached_multiapp_pos_ids[i]),
                         pp_values[_cached_multiapp_pos_ids[i]]);
            ++i;
          }
        }
      }
      solution.close();
      break;
    }
  }
}
