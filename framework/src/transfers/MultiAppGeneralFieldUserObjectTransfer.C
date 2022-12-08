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
  params.addRequiredParam<UserObjectName>(
      "source_user_object",
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
}

void
MultiAppGeneralFieldUserObjectTransfer::prepareEvaluationOfInterpValues(const VariableName & /* var_name */)
{
  _local_bboxes.clear();
  extractLocalFromBoundingBoxes(_local_bboxes);
}

void
MultiAppGeneralFieldUserObjectTransfer::evaluateInterpValues(
    const std::vector<Point> & incoming_points, std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  evaluateInterpValuesWithUserObjects(
      _local_bboxes, incoming_points, outgoing_vals);
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
    // Loop until we've found the lowest-ranked app that actually contains
    // the quadrature point.
    // NOTE We are missing some overlap detection by accepting the first result
    for (MooseIndex(_from_problems.size()) i_from = 0;
         i_from < _from_problems.size() &&
         (outgoing_vals[i_pt].first == GeneralFieldTransfer::BetterOutOfMeshValue ||
          _greedy_search);
         ++i_from)
    {
      if (!acceptPointInOriginMesh(i_from, local_bboxes, pt))
        continue;
      else
      {
        // Get user object from the local problem
        const UserObject & user_object =
            _from_problems[i_from]->getUserObjectBase(_user_object_name);

        // Use spatial value routine to compute the origin value to transfer
        auto val = user_object.spatialValue(pt - _from_positions[i_from]);

        // Look for overlaps. The check is not active outside of greedy search because in that
        // case we accept the first value from the lowest ranked process
        if (_greedy_search && val != GeneralFieldTransfer::BetterOutOfMeshValue &&
            outgoing_vals[i_pt].first != GeneralFieldTransfer::BetterOutOfMeshValue)
          _num_overlaps++;

        // Assign value
        outgoing_vals[i_pt].first = val;
        if (outgoing_vals[i_pt].first == GeneralFieldTransfer::BetterOutOfMeshValue)
          outgoing_vals[i_pt].second = GeneralFieldTransfer::BetterOutOfMeshValue;
        else
          outgoing_vals[i_pt].second = 1;
      }
    }

    // Move to next point
    i_pt++;
  }
}
