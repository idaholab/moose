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
#include "LineSegment.h"
#include "NearestNodeLocator.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "GeometricSearchData.h"
#include "PenetrationThread.h"
#include "Moose.h"

std::string _PLBoundaryFuser(unsigned int boundary1, unsigned int boundary2)
{
  std::stringstream ss;

  ss << boundary1 << "to" << boundary2;

  return ss.str();
}

PenetrationLocator::PenetrationLocator(SubProblem & subproblem, GeometricSearchData & /*geom_search_data*/, MooseMesh & mesh, const unsigned int master_id, const unsigned int slave_id, Order order, NearestNodeLocator & nearest_node) :
    Restartable(_PLBoundaryFuser(master_id, slave_id), "PenetrationLocator", subproblem, 0),
    _subproblem(subproblem),
    _mesh(mesh),
    _master_boundary(master_id),
    _slave_boundary(slave_id),
    _fe_type(order),
    _nearest_node(nearest_node),
    _penetration_info(declareRestartableDataWithContext<std::map<unsigned int, PenetrationInfo *> >("penetration_info", &_mesh)),
    _has_penetrated(declareRestartableData<std::set<unsigned int> >("has_penetrated")),
    _locked_this_step(declareRestartableData<std::map<unsigned int, unsigned> >("locked_this_step")),
    _unlocked_this_step(declareRestartableData<std::map<unsigned int, unsigned> >("unlocked_this_step")),
    _lagrange_multiplier(declareRestartableData<std::map<unsigned int, Real> >("lagrange_multiplier")),
    _update_location(declareRestartableData<bool>("update_location", true)),
    _tangential_tolerance(0.0),
    _do_normal_smoothing(false),
    _normal_smoothing_distance(0.0),
    _normal_smoothing_method(NSM_EDGE_BASED)
{
  // Preconstruct an FE object for each thread we're going to use
  // This is a time savings so that the thread objects don't do this themselves multiple times
  _fe.resize(libMesh::n_threads());
  for(unsigned int i=0; i < libMesh::n_threads(); i++)
    _fe[i] = FEBase::build(_mesh.dimension()-1, _fe_type).release();

  if (_normal_smoothing_method == NSM_NODAL_NORMAL_BASED)
  {
    if (!((_subproblem.hasVariable("nodal_normal_x")) &&
          (_subproblem.hasVariable("nodal_normal_y")) &&
          (_subproblem.hasVariable("nodal_normal_z"))))
    {
      mooseError("To use nodal-normal-based smoothing, the nodal_normal_x, nodal_normal_y, and nodal_normal_z variables must exist.  Are you missing the [NodalNormals] block?");
    }
  }
}

PenetrationLocator::~PenetrationLocator()
{
  for(unsigned int i=0; i < libMesh::n_threads(); i++)
    delete _fe[i];

  for (std::map<unsigned int, PenetrationInfo *>::iterator it = _penetration_info.begin(); it != _penetration_info.end(); ++it)
    delete it->second;
}

void
PenetrationLocator::detectPenetration()
{
  Moose::perf_log.push("detectPenetration()","Solve");

  // Data structures to hold the element boundary information
  std::vector< unsigned int > elem_list;
  std::vector< unsigned short int > side_list;
  std::vector< short int > id_list;

  // Retrieve the Element Boundary data structures from the mesh
  _mesh.buildSideList(elem_list, side_list, id_list);

  // Grab the slave nodes we need to worry about from the NearestNodeLocator
  NodeIdRange & slave_node_range = _nearest_node.slaveNodeRange();

  PenetrationThread pt(_subproblem,
                       _mesh,
                       _master_boundary,
                       _slave_boundary,
                       _penetration_info,
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

  Moose::perf_log.pop("detectPenetration()","Solve");
}

void
PenetrationLocator::reinit()
{
  _penetration_info.clear();
  _has_penetrated.clear();
  _locked_this_step.clear();
  _unlocked_this_step.clear();
  _lagrange_multiplier.clear();

  detectPenetration();
}

Real
PenetrationLocator::penetrationDistance(unsigned int node_id)
{
  PenetrationInfo * info = _penetration_info[node_id];

  if (info)
    return info->_distance;
  else
    return 0;
}


RealVectorValue
PenetrationLocator::penetrationNormal(unsigned int node_id)
{
  std::map<unsigned int, PenetrationInfo *>::const_iterator found_it( _penetration_info.find(node_id) );

  if (found_it != _penetration_info.end())
    return found_it->second->_normal;
  else
    return RealVectorValue(0, 0, 0);
}

void
PenetrationLocator::setUpdate( bool update )
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
    mooseError("Invalid normal_smoothing_method: "<<nsmString);
  _do_normal_smoothing = true;
}

void
PenetrationLocator::saveContactStateVars()
{
  std::map<unsigned int, PenetrationInfo *>::iterator it( _penetration_info.begin() );
  const std::map<unsigned int, PenetrationInfo *>::iterator it_end( _penetration_info.end() );
  for ( ; it != it_end; ++it )
  {
    if (it->second != NULL)
    {
      it->second->_contact_force_old = it->second->_contact_force;
      it->second->_accumulated_slip_old = it->second->_accumulated_slip;
      it->second->_frictional_energy_old = it->second->_frictional_energy;
    }
  }
}
