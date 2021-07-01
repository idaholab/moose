//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenetrationLocator.h"

#include "ArbitraryQuadrature.h"
#include "Conversion.h"
#include "GeometricSearchData.h"
#include "LineSegment.h"
#include "MooseMesh.h"
#include "NearestNodeLocator.h"
#include "PenetrationThread.h"
#include "SubProblem.h"
#include "MooseApp.h"

PenetrationLocator::PenetrationLocator(SubProblem & subproblem,
                                       GeometricSearchData & /*geom_search_data*/,
                                       MooseMesh & mesh,
                                       const unsigned int primary_id,
                                       const unsigned int secondary_id,
                                       Order order,
                                       NearestNodeLocator & nearest_node)
  : Restartable(subproblem.getMooseApp(),
                Moose::stringify(primary_id) + "to" + Moose::stringify(secondary_id),
                "PenetrationLocator",
                0),
    PerfGraphInterface(subproblem.getMooseApp().perfGraph(),
                       "PenetrationLocator_" + Moose::stringify(primary_id) + "_" +
                           Moose::stringify(secondary_id)),
    _subproblem(subproblem),
    _mesh(mesh),
    _primary_boundary(primary_id),
    _secondary_boundary(secondary_id),
    _fe_type(order),
    _nearest_node(nearest_node),
    _penetration_info(declareRestartableDataWithContext<std::map<dof_id_type, PenetrationInfo *>>(
        "penetration_info", &_mesh)),
    _has_penetrated(declareRestartableData<std::set<dof_id_type>>("has_penetrated")),
    _check_whether_reasonable(true),
    _update_location(declareRestartableData<bool>("update_location", true)),
    _tangential_tolerance(0.0),
    _do_normal_smoothing(false),
    _normal_smoothing_distance(0.0),
    _normal_smoothing_method(NSM_EDGE_BASED),
    _patch_update_strategy(_mesh.getPatchUpdateStrategy())
{
  // Preconstruct an FE object for each thread we're going to use and for each lower-dimensional
  // element
  // This is a time savings so that the thread objects don't do this themselves multiple times
  _fe.resize(libMesh::n_threads());
  for (unsigned int i = 0; i < libMesh::n_threads(); i++)
  {
    unsigned int n_dims = _mesh.dimension();
    _fe[i].resize(n_dims + 1);
    for (unsigned int dim = 0; dim <= n_dims; ++dim)
      _fe[i][dim] = FEBase::build(dim, _fe_type).release();
  }

  if (_normal_smoothing_method == NSM_NODAL_NORMAL_BASED)
  {
    if (!((_subproblem.hasVariable("nodal_normal_x")) &&
          (_subproblem.hasVariable("nodal_normal_y")) &&
          (_subproblem.hasVariable("nodal_normal_z"))))
    {
      mooseError(
          "To use nodal-normal-based smoothing, the nodal_normal_x, nodal_normal_y, and "
          "nodal_normal_z variables must exist.  Are you missing the \\[NodalNormals\\] block?");
    }
  }
}

PenetrationLocator::~PenetrationLocator()
{
  for (unsigned int i = 0; i < libMesh::n_threads(); i++)
    for (unsigned int dim = 0; dim < _fe[i].size(); dim++)
      delete _fe[i][dim];

  for (auto & it : _penetration_info)
    delete it.second;
}

void
PenetrationLocator::detectPenetration()
{
  TIME_SECTION("detectPenetration", 3, "Detecting Penetration");

  // Get list of boundary (elem, side, id) tuples.
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> bc_tuples =
      _mesh.buildActiveSideList();

  // Grab the secondary nodes we need to worry about from the NearestNodeLocator
  NodeIdRange & secondary_node_range = _nearest_node.secondaryNodeRange();

  PenetrationThread pt(_subproblem,
                       _mesh,
                       _primary_boundary,
                       _secondary_boundary,
                       _penetration_info,
                       _check_whether_reasonable,
                       _update_location,
                       _tangential_tolerance,
                       _do_normal_smoothing,
                       _normal_smoothing_distance,
                       _normal_smoothing_method,
                       _fe,
                       _fe_type,
                       _nearest_node,
                       _mesh.nodeToElemMap(),
                       bc_tuples);

  Threads::parallel_reduce(secondary_node_range, pt);

  std::vector<dof_id_type> recheck_secondary_nodes = pt._recheck_secondary_nodes;

  // Update the patch for the secondary nodes in recheck_secondary_nodes and re-run penetration
  // thread on these nodes at every nonlinear iteration if patch update strategy is set to
  // "iteration".
  if (recheck_secondary_nodes.size() > 0 && _patch_update_strategy == Moose::Iteration &&
      _subproblem.currentlyComputingJacobian())
  {
    // Update the patch for this subset of secondary nodes and calculate the nearest neighbor_nodes
    _nearest_node.updatePatch(recheck_secondary_nodes);

    // Re-run the penetration thread to see if these nodes are in contact with the updated patch
    NodeIdRange recheck_secondary_node_range(
        recheck_secondary_nodes.begin(), recheck_secondary_nodes.end(), 1);

    Threads::parallel_reduce(recheck_secondary_node_range, pt);
  }

  if (recheck_secondary_nodes.size() > 0 && _patch_update_strategy != Moose::Iteration &&
      _subproblem.currentlyComputingJacobian())
    mooseDoOnce(mooseWarning("Warning in PenetrationLocator. Penetration is not "
                             "detected for one or more secondary nodes. This could be because "
                             "those secondary nodes simply do not project to faces on the primary "
                             "surface. However, this could also be because contact should be "
                             "enforced on those nodes, but the faces that they project to "
                             "are outside the contact patch, which will give an erroneous "
                             "result. Use appropriate options for 'patch_size' and "
                             "'patch_update_strategy' in the Mesh block to avoid this issue. "
                             "Setting 'patch_update_strategy=iteration' is recommended because "
                             "it completely avoids this potential issue. Also note that this "
                             "warning is printed only once, so a similar situation could occur "
                             "multiple times during the simulation but this warning is printed "
                             "only at the first occurrence."));
}

void
PenetrationLocator::reinit()
{
  TIME_SECTION("reinit", 3, "Reinitializing PenetrationLocator");

  // Delete the PenetrationInfo objects we own before clearing the
  // map, or we have a memory leak.
  for (auto & it : _penetration_info)
    delete it.second;

  _penetration_info.clear();

  _has_penetrated.clear();

  detectPenetration();
}

Real
PenetrationLocator::penetrationDistance(dof_id_type node_id)
{
  PenetrationInfo * info = _penetration_info[node_id];

  if (info)
    return info->_distance;
  else
    return 0;
}

RealVectorValue
PenetrationLocator::penetrationNormal(dof_id_type node_id)
{
  std::map<dof_id_type, PenetrationInfo *>::const_iterator found_it =
      _penetration_info.find(node_id);

  if (found_it != _penetration_info.end())
    return found_it->second->_normal;
  else
    return RealVectorValue(0, 0, 0);
}

void
PenetrationLocator::setCheckWhetherReasonable(bool state)
{
  _check_whether_reasonable = state;
}

void
PenetrationLocator::setUpdate(bool update)
{
  _update_location = update;
}

void
PenetrationLocator::setTangentialTolerance(Real tangential_tolerance)
{
  _tangential_tolerance = tangential_tolerance;
}

void
PenetrationLocator::setNormalSmoothingDistance(Real normal_smoothing_distance)
{
  _normal_smoothing_distance = normal_smoothing_distance;
  if (_normal_smoothing_distance > 0.0)
    _do_normal_smoothing = true;
}

void
PenetrationLocator::setNormalSmoothingMethod(std::string nsmString)
{
  if (nsmString == "edge_based")
    _normal_smoothing_method = NSM_EDGE_BASED;
  else if (nsmString == "nodal_normal_based")
    _normal_smoothing_method = NSM_NODAL_NORMAL_BASED;
  else
    mooseError("Invalid normal_smoothing_method: ", nsmString);
  _do_normal_smoothing = true;
}
