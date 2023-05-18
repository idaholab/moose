//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldUserObjectTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "UserObject.h"
#include "Positions.h"

#include "libmesh/system.h"
#include "libmesh/mesh_function.h"

registerMooseObject("MooseApp", MultiAppGeneralFieldUserObjectTransfer);

InputParameters
MultiAppGeneralFieldUserObjectTransfer::validParams()
{
  InputParameters params = MultiAppGeneralFieldTransfer::validParams();
  params.addClassDescription(
      "Transfers user object spatial evaluations from an origin app onto a variable in the target "
      "application.");

  params.suppressParameter<std::vector<VariableName>>("source_variable");
  params.addRequiredParam<UserObjectName>("source_user_object",
                                          "The UserObject you want to transfer values from. "
                                          "It must implement the SpatialValue() class routine");

  // Blanket ban on origin boundary restriction. User objects tend to extend beyond boundaries,
  // and be able to be evaluated within a volume rather than only on a boundary
  // This could be re-enabled for spatial user objects that are only defined on boundaries
  params.suppressParameter<std::vector<BoundaryName>>("from_boundaries");
  return params;
}

MultiAppGeneralFieldUserObjectTransfer::MultiAppGeneralFieldUserObjectTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldTransfer(parameters),
    _user_object_name(getParam<UserObjectName>("source_user_object"))
{
  if (_to_var_names.size() > 1)
    paramError("variable", "Only one variable at a time is supported by this transfer");

  // Block restriction does not make sense if we're ok with extrapolating
  if (isParamValid("from_blocks") && !_source_app_must_contain_point &&
      !parameters.isParamSetByUser("extrapolation_constant"))
    paramError("from_app_must_contain_point",
               "Source block restriction cannot be used at the same type as allowing extrapolation"
               " of values for a user object transfer (with 'from_app_must_contain_point=false') "
               " unless an extrapolation constant is provided (with 'extrapolation_constant')");

  // Nearest point isn't well defined for sending app-based data from main app to a multiapp
  if (_nearest_positions_obj && isParamValid("to_multi_app") && !isParamValid("from_multi_app"))
    paramError("use_nearest_position",
               "Cannot use nearest-position algorithm when sending from the main application");
}

void
MultiAppGeneralFieldUserObjectTransfer::prepareEvaluationOfInterpValues(
    const unsigned int /* var_index */)
{
  _local_bboxes.clear();
  extractLocalFromBoundingBoxes(_local_bboxes);
}

void
MultiAppGeneralFieldUserObjectTransfer::evaluateInterpValues(
    const std::vector<Point> & incoming_points, std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  evaluateInterpValuesWithUserObjects(_local_bboxes, incoming_points, outgoing_vals);
}

void
MultiAppGeneralFieldUserObjectTransfer::evaluateInterpValuesWithUserObjects(
    const std::vector<BoundingBox> & local_bboxes,
    const std::vector<Point> & incoming_points,
    std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  dof_id_type i_pt = 0;
  for (auto & pt : incoming_points)
  {
    bool point_found = false;
    if (_nearest_positions_obj)
      outgoing_vals[i_pt].second = GeneralFieldTransfer::BetterOutOfMeshValue;

    // Loop on all local origin problems until:
    // - we've found the point in an app and the value at that point is valid
    // - or if looking for conflicts between apps, we must check them all
    for (MooseIndex(_from_problems.size()) i_from = 0;
         i_from < _from_problems.size() &&
         (!point_found || _search_value_conflicts || _nearest_positions_obj);
         ++i_from)
    {
      // Check spatial restrictions
      if (!acceptPointInOriginMesh(i_from, local_bboxes, pt))
        continue;
      else
      {
        // Get user object from the local problem
        const UserObject & user_object =
            _from_problems[i_from]->getUserObjectBase(_user_object_name);

        // Use spatial value routine to compute the origin value to transfer
        auto val = user_object.spatialValue(pt - _from_positions[i_from]);

        // Get nearest position (often a subapp position) for the target point
        // We want values from the child app that is closest to the same position as the target
        Point nearest_position_source;
        if (_nearest_positions_obj)
        {
          const bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;
          const Point nearest_position = _nearest_positions_obj->getNearestPosition(pt, initial);
          nearest_position_source =
              _nearest_positions_obj->getNearestPosition(_from_positions[i_from], initial);

          // Source (usually app position) is not closest to the same positions as the target, dont
          // send values
          if (nearest_position != nearest_position_source)
            continue;
        }

        // Look for overlaps. The check is not active outside of overlap search because in that
        // case we accept the first value from the lowest ranked process
        // NOTE: There is no guarantee this will be the final value used among all problems
        //       but we register an overlap as soon as two values are possible from this rank
        if (detectConflict(val,
                           outgoing_vals[i_pt].first,
                           _nearest_positions_obj ? (pt - nearest_position_source).norm() : 1,
                           outgoing_vals[i_pt].second))
          registerConflict(i_from, 0, pt - _from_positions[i_from], 1, true);

        // No need to consider decision factors if value is invalid
        if (val == GeneralFieldTransfer::BetterOutOfMeshValue)
          continue;
        else
          point_found = true;

        // Assign value
        if (!_nearest_positions_obj)
        {
          outgoing_vals[i_pt].first = val;
          outgoing_vals[i_pt].second = 1;
        }
        else if ((pt - nearest_position_source).norm() < outgoing_vals[i_pt].second)
        {
          outgoing_vals[i_pt].first = val;
          outgoing_vals[i_pt].second = (pt - nearest_position_source).norm();
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
