//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"

#include <set>
#include <map>
#include <memory>

// Forward declarations
class MooseMesh;

namespace libMesh
{
class Elem;
class PointLocatorBase;
class Point;
}

/**
 * The DiracKernelInfo object is a place where all the Dirac points
 * added by different DiracKernels are collected.  It is used, for
 * example, by the FEProblemBase class to determine if finite element data
 * needs to be recomputed on a given element.
 */
class DiracKernelInfo
{
public:
  DiracKernelInfo();
  virtual ~DiracKernelInfo();

public:
  /**
   * Adds a point source
   * @param elem Pointer to the geometric element in which the point is located
   * @param p The (x,y,z) location of the Dirac point
   */
  void addPoint(const Elem * elem, const Point & p);

  /**
   * Remove all of the current points and elements.
   */
  void clearPoints();

  /**
   * Return true if we have Point 'p' in Element 'elem'
   */
  bool hasPoint(const Elem * elem, const Point & p);

  /**
   * Returns a writeable reference to the _elements container.
   */
  std::set<const Elem *> & getElements() { return _elements; }

  typedef std::map<const Elem *, std::pair<std::vector<Point>, std::vector<unsigned int>>>
      MultiPointMap;

  /**
   * Returns a writeable reference to the _points container.
   */
  MultiPointMap & getPoints() { return _points; }

  /**
   * Called during FEProblemBase::meshChanged() to update the PointLocator
   * object used by the DiracKernels.
   */
  void updatePointLocator(const MooseMesh & mesh);

  /**
   * Used by client DiracKernel classes to determine the Elem in which
   * the Point p resides.  Uses the PointLocator owned by this object.
   */
  const Elem *
  findPoint(const Point & p, const MooseMesh & mesh, const std::set<SubdomainID> & blocks);

protected:
  /**
   * Check if two points are equal with respect to a tolerance
   */
  bool pointsFuzzyEqual(const Point &, const Point &);

  /// The list of elements that need distributions.
  std::set<const Elem *> _elements;

  /// The list of physical xyz Points that need to be evaluated in each element.
  MultiPointMap _points;

  /// The DiracKernelInfo object manages a PointLocator object which is used
  /// by all DiracKernels to find Points.  It needs to be centrally managed and it
  /// also needs to be rebuilt in FEProblemBase::meshChanged() to work with Mesh
  /// adaptivity.
  std::unique_ptr<libMesh::PointLocatorBase> _point_locator;

  /// threshold distance squared below which two points are considered identical
  const Real _point_equal_distance_sq;
};
