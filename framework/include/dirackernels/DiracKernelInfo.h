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
#include "libmesh/elem.h"
#include "libmesh/point.h"

#include <set>
#include <map>

/**
 * The DiracKernelInfo object is a place where all the Dirac points
 * added by different DiracKernels are collected.  It is used, for
 * example, by the FEProblem class to determine if finite element data
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

  /**
   * Returns a writeable reference to the _points container.
   */
  std::map<const Elem *, std::vector<Point> > & getPoints() { return _points; }

protected:
  /// The list of elements that need distributions.
  std::set<const Elem *> _elements;

  /// The list of physical xyz Points that need to be evaluated in each element.
  std::map<const Elem *, std::vector<Point> > _points;
};

#endif //DIRACKERNELINFO_H
