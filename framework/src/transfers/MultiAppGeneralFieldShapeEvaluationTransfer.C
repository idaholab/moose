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
  return params;
}

MultiAppGeneralFieldShapeEvaluationTransfer::MultiAppGeneralFieldShapeEvaluationTransfer(
    const InputParameters & parameters)
  : MultiAppGeneralFieldTransfer(parameters)
{
}

void
MultiAppGeneralFieldShapeEvaluationTransfer::prepareEvaluationOfInterpValues(
    const VariableName & var_name)
{
  _local_bboxes.clear();
  extractLocalFromBoundingBoxes(_local_bboxes);

  _local_meshfuns.clear();
  buildMeshFunctions(var_name, _local_meshfuns);
}

void
MultiAppGeneralFieldShapeEvaluationTransfer::buildMeshFunctions(
    const VariableName & var_name, std::vector<std::shared_ptr<MeshFunction>> & local_meshfuns)
{
  // Construct a local mesh function for each origin problem
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariableFEBase & from_var = from_problem.getVariable(
        0, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());

    std::shared_ptr<MeshFunction> from_func;
    from_func.reset(new MeshFunction(getEquationSystem(from_problem, _displaced_source_mesh),
                                     *from_sys.current_local_solution,
                                     from_sys.get_dof_map(),
                                     from_var_num));
    from_func->init();
    from_func->enable_out_of_mesh_mode(GeneralFieldTransfer::BetterOutOfMeshValue);
    local_meshfuns.push_back(from_func);
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
    const std::vector<std::shared_ptr<MeshFunction>> & local_meshfuns,
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
      if (local_bboxes[i_from].contains_point(pt))
      {
        // Check block restriction (boundary restriction is not supported)
        if (_from_blocks.size() && !hasBlocks(_from_blocks, i_from, pt))
          continue;

        // Use mesh function to compute interpolation values
        auto val = (*local_meshfuns[i_from])(pt - _from_positions[i_from]);

        // Look for overlaps
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
