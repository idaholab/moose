/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PenetrationLocator.h"

#include "ArbitraryQuadrature.h"
#include "Conversion.h"
#include "GeometricSearchData.h"
#include "LineSegment.h"
#include "MooseMesh.h"
#include "NearestNodeLocator.h"
#include "PenetrationThread.h"
#include "SubProblem.h"

PenetrationLocator::PenetrationLocator(SubProblem & subproblem,
                                       GeometricSearchData & /*geom_search_data*/,
                                       MooseMesh & mesh,
                                       const unsigned int master_id,
                                       const unsigned int slave_id,
                                       Order order,
                                       NearestNodeLocator & nearest_node)
  : Restartable(Moose::stringify(master_id) + "to" + Moose::stringify(slave_id),
                "PenetrationLocator",
                subproblem,
                0),
    _subproblem(subproblem),
    _mesh(mesh),
    _master_boundary(master_id),
    _slave_boundary(slave_id),
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
    _normal_smoothing_method(NSM_EDGE_BASED)
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
      mooseError("To use nodal-normal-based smoothing, the nodal_normal_x, nodal_normal_y, and "
                 "nodal_normal_z variables must exist.  Are you missing the [NodalNormals] block?");
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
  Moose::perf_log.push("detectPenetration()", "Execution");

  // Data structures to hold the element boundary information
  std::vector<dof_id_type> elem_list;
  std::vector<unsigned short int> side_list;
  std::vector<boundary_id_type> id_list;

  // Retrieve the Element Boundary data structures from the mesh
  _mesh.buildSideList(elem_list, side_list, id_list);

  // Grab the slave nodes we need to worry about from the NearestNodeLocator
  NodeIdRange & slave_node_range = _nearest_node.slaveNodeRange();

  PenetrationThread pt(_subproblem,
                       _mesh,
                       _master_boundary,
                       _slave_boundary,
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
                       elem_list,
                       side_list,
                       id_list);

  Threads::parallel_reduce(slave_node_range, pt);

  Moose::perf_log.pop("detectPenetration()", "Execution");
}

void
PenetrationLocator::reinit()
{
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
