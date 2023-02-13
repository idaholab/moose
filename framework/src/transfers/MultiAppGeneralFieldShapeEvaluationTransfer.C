//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldShapeEvaluationTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"

#include "libmesh/system.h"
#include "libmesh/mesh_function.h"

registerMooseObject("MooseApp", MultiAppGeneralFieldShapeEvaluationTransfer);

InputParameters
MultiAppGeneralFieldShapeEvaluationTransfer::validParams()
{
  InputParameters params = MultiAppGeneralFieldTransfer::validParams();
  params.addClassDescription(
      "Transfers field data at the MultiApp position using the finite element shape "
      "functions from the origin application.");

  // Blanket ban on origin boundary restriction. Most shape functions have their support extend
  // outside the boundary. For a true face variable, this parameter would make sense again
  params.suppressParameter<std::vector<BoundaryName>>("from_boundaries");
  // Shape function evaluations return an invalid value outside an app's domain anyway
  params.suppressParameter<bool>("from_app_must_contain_point");

  return params;
}

MultiAppGeneralFieldShapeEvaluationTransfer::MultiAppGeneralFieldShapeEvaluationTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldTransfer(parameters)
{
}

void
MultiAppGeneralFieldShapeEvaluationTransfer::prepareEvaluationOfInterpValues(
    const unsigned int var_index)
{
  _local_bboxes.clear();
  extractLocalFromBoundingBoxes(_local_bboxes);

  _local_meshfuns.clear();
  buildMeshFunctions(var_index, _local_meshfuns);
}

void
MultiAppGeneralFieldShapeEvaluationTransfer::buildMeshFunctions(
    const unsigned int var_index, std::vector<MeshFunction> & local_meshfuns)
{
  local_meshfuns.reserve(_from_problems.size());

  // Construct a local mesh function for each origin problem
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariableFieldBase & from_var =
        from_problem.getVariable(0,
                                 _from_var_names[var_index],
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_ANY);

    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(getFromVarName(var_index));

    local_meshfuns.emplace_back(getEquationSystem(from_problem, _displaced_source_mesh),
                                *from_sys.current_local_solution,
                                from_sys.get_dof_map(),
                                from_var_num);
    local_meshfuns.back().init();
    local_meshfuns.back().enable_out_of_mesh_mode(GeneralFieldTransfer::BetterOutOfMeshValue);
  }
}

void
MultiAppGeneralFieldShapeEvaluationTransfer::evaluateInterpValues(
    const std::vector<Point> & incoming_points, std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  evaluateInterpValuesWithMeshFunctions(
      _local_bboxes, _local_meshfuns, incoming_points, outgoing_vals);
}

void
MultiAppGeneralFieldShapeEvaluationTransfer::evaluateInterpValuesWithMeshFunctions(
    const std::vector<BoundingBox> & local_bboxes,
    std::vector<MeshFunction> & local_meshfuns,
    const std::vector<Point> & incoming_points,
    std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  dof_id_type i_pt = 0;
  for (auto & pt : incoming_points)
  {
    bool point_found = false;
    if (_use_nearest_app)
      outgoing_vals[i_pt].second = GeneralFieldTransfer::BetterOutOfMeshValue;

    // Loop on all local origin problems until:
    // - we've found the point in an app and the value at that point is valid
    // - or if looking for conflicts between apps, we must check them all
    // - or if looking for the nearest app, we also check them all
    for (MooseIndex(_from_problems.size()) i_from = 0;
         i_from < _from_problems.size() &&
         (!point_found || _search_value_conflicts || _use_nearest_app);
         ++i_from)
    {
      // Check spatial restrictions
      if (!acceptPointInOriginMesh(i_from, local_bboxes, pt))
        continue;
      else
      {
        // Use mesh function to compute interpolation values
        auto val = (local_meshfuns[i_from])(pt - _from_positions[i_from]);

        // Look for overlaps. The check is not active outside of overlap search because in that
        // case we accept the first value from the lowest ranked process
        // NOTE: There is no guarantee this will be the final value used among all problems
        //       but for shape evaluation we really do expect only one value to even be valid
        if (detectConflict(val,
                           outgoing_vals[i_pt].first,
                           _use_nearest_app ? (pt - _from_positions[i_from]).norm() : 1,
                           outgoing_vals[i_pt].second))
          registerConflict(i_from, 0, pt - _from_positions[i_from], 1, true);

        // No need to consider decision factors if value is invalid
        if (val == GeneralFieldTransfer::BetterOutOfMeshValue)
          continue;
        else
          point_found = true;

        // Assign value
        if (!_use_nearest_app)
        {
          outgoing_vals[i_pt].first = val;
          outgoing_vals[i_pt].second = 1;
        }
        else if ((pt - _from_positions[i_from]).norm() < outgoing_vals[i_pt].second)
        {
          outgoing_vals[i_pt].first = val;
          outgoing_vals[i_pt].second = (pt - _from_positions[i_from]).norm();
        }
      }
    }

    if (!point_found)
      outgoing_vals[i_pt] = {GeneralFieldTransfer::BetterOutOfMeshValue,
                             GeneralFieldTransfer::BetterOutOfMeshValue};

    // Move to next point
    i_pt++;
  }
}
