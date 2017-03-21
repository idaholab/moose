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

#ifndef DIRACKERNELINFO_H
#define DIRACKERNELINFO_H

#include "Moose.h"
#include "MooseArray.h"

// libMesh
#include "libmesh/point.h"

#include <set>
#include <map>

// Forward declarations
class MooseMesh;

namespace libMesh
{
class Elem;
class PointLocatorBase;
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
  void addPoint(const Elem * elem, Point p);

  /**
   * Remove all of the current points and elements.
   */
  void clearPoints();

  /**
   * Return true if we have Point 'p' in Element 'elem'
   */
  bool hasPoint(const Elem * elem, Point p);

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
  const Elem * findPoint(Point p, const MooseMesh & mesh);

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
  std::unique_ptr<PointLocatorBase> _point_locator;

  /// threshold distance squared below which two points are considered identical
  const Real _point_equal_distance_sq;
};

#endif // DIRACKERNELINFO_H
