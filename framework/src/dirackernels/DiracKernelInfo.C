//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiracKernelInfo.h"
#include "MooseMesh.h"

// LibMesh
#include "libmesh/point_locator_base.h"
#include "libmesh/elem.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/point.h"

using namespace libMesh;

DiracKernelInfo::DiracKernelInfo()
  : _point_locator(), _point_equal_distance_sq(libMesh::TOLERANCE * libMesh::TOLERANCE)
{
}

DiracKernelInfo::~DiracKernelInfo() {}

void
DiracKernelInfo::addPoint(const Elem * elem, const Point & p)
{
  _elements.insert(elem);

  std::pair<std::vector<Point>, std::vector<unsigned int>> & multi_point_list = _points[elem];

  const unsigned int npoint = multi_point_list.first.size();
  mooseAssert(npoint == multi_point_list.second.size(),
              "Different sizes for location and multiplicity data");

  for (unsigned int i = 0; i < npoint; ++i)
    if (pointsFuzzyEqual(multi_point_list.first[i], p))
    {
      // a point at the same (within a tolerance) location as p exists, increase its multiplicity
      multi_point_list.second[i]++;
      return;
    }

  // no prior point found at this location, add it with a multiplicity of one
  multi_point_list.first.push_back(p);
  multi_point_list.second.push_back(1);
}

void
DiracKernelInfo::clearPoints()
{
  _elements.clear();
  _points.clear();
}

bool
DiracKernelInfo::hasPoint(const Elem * elem, const Point & p)
{
  std::vector<Point> & point_list = _points[elem].first;

  for (const auto & pt : point_list)
    if (pointsFuzzyEqual(pt, p))
      return true;

  // If we haven't found it, we don't have it.
  return false;
}

void
DiracKernelInfo::updatePointLocator(const MooseMesh & mesh)
{
  // Note: we could update the PointLocator *every* time we call this
  // function, but that may introduce an unacceptable overhead in
  // problems which don't need a PointLocator at all.  This issue will
  // most likely become a moot point when we eventually add a shared
  // "CachingPointLocator" to MOOSE.
  // _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, mesh);

  // Construct the PointLocator object if *any* processors have Dirac
  // points.  Note: building a PointLocator object is a parallel_only()
  // function, so this is an all-or-nothing thing.
  unsigned pl_needs_rebuild = _elements.size();
  mesh.comm().max(pl_needs_rebuild);

  if (pl_needs_rebuild)
  {
    // PointLocatorBase::build() is a parallel_only function!  So we
    // can't skip building it just becuase our local _elements is
    // empty, it might be non-empty on some other processor!
    _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, mesh);

    // We may be querying for points which are not in the semilocal
    // part of a distributed mesh.
    _point_locator->enable_out_of_mesh_mode();
  }
  else
  {
    // There are no elements with Dirac points, but we have been
    // requested to update the PointLocator so we have to assume the
    // old one is invalid.  Therefore we reset it to NULL... however
    // adding this line causes the code to hang because it triggers
    // the PointLocator to be rebuilt in a non-parallel-only segment
    // of the code later... so it's commented out for now even though
    // it's probably the right behavior.
    // _point_locator.reset(NULL);
  }
}

const Elem *
DiracKernelInfo::findPoint(const Point & p,
                           const MooseMesh & mesh,
                           const std::set<SubdomainID> & blocks)
{
  // If the PointLocator has never been created, do so now.  NOTE - WE
  // CAN'T DO THIS if findPoint() is only called on some processors,
  // PointLocatorBase::build() is a 'parallel_only' method!
  if (_point_locator.get() == NULL)
  {
    _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, mesh);
    _point_locator->enable_out_of_mesh_mode();
  }

  // Check that the PointLocator is ready to start locating points.
  // So far I do not have any tests that trip this...
  if (_point_locator->initialized() == false)
    mooseError("Error, PointLocator is not initialized!");

  // Note: The PointLocator object returns NULL when the Point is not
  // found within the Mesh.  This is not considered to be an error as
  // far as the DiracKernels are concerned: sometimes the Mesh moves
  // out from the Dirac point entirely and in that case the Point just
  // gets "deactivated".
  const Elem * elem = (*_point_locator)(p, &blocks);

  // The processors may not agree on which Elem the point is in.  This
  // can happen if a Dirac point lies on the processor boundary, and
  // two or more neighboring processors think the point is in the Elem
  // on *their* side.
  dof_id_type elem_id = elem ? elem->id() : DofObject::invalid_id;

  // We are going to let the element with the smallest ID "win", all other
  // procs will return NULL.
  dof_id_type min_elem_id = elem_id;
  mesh.comm().min(min_elem_id);

  return min_elem_id == elem_id ? elem : NULL;
}

bool
DiracKernelInfo::pointsFuzzyEqual(const Point & a, const Point & b)
{
  const Real dist_sq = (a - b).norm_sq();
  return dist_sq < _point_equal_distance_sq;
}
