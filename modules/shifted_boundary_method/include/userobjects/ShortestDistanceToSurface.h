//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "ThreadedGeneralUserObject.h"

/**
 * User object that computes the shortest distance to one or more surfaces
 * defined by function- and/or mesh-based distance strategies.
 *
 * This object only answers distance queries on demand; it does not store or
 * build up any results of its own. Other objects call distanceVector()/
 * trueNormal()/... directly, and there is no per-element work to do, so an
 * ElementUserObject's element loop would not be used here. For that reason it
 * is a ThreadedGeneralUserObject rather than an element-based user object.
 *
 * A ThreadedGeneralUserObject is constructed once per thread. Each copy stores
 * its own per-thread Function pointers; parsed functions mutate internal state
 * during evaluation, so sharing the TID=0 copy across threads can race and
 * produce incorrect distances. The per-thread copies avoid that race.
 */
class ShortestDistanceToSurface : public ThreadedGeneralUserObject
{
public:
  static InputParameters validParams();
  ShortestDistanceToSurface(const InputParameters & parameters);
  void initialize() override {}
  void execute() override {}
  void finalize() override {}

  // This object answers queries on demand and stores no results, so there is
  // nothing to combine across threads.
  void threadJoin(const UserObject &) override {}

  /// @brief Get the SBMSurfaceMeshBuilder user objects
  const std::vector<const Function *> & getDistanceFuncs() const { return _distance_functions; }

  /// Return the closest distance vector from point pt to the boundaries
  RealVectorValue distanceVector(const Point & pt) const;

  /// Return the true normal vector at the closest point on the boundaries
  RealVectorValue trueNormal(const Point & pt) const;

  /// A distance vector from a point \p pt to a specific surface specified by index \p idx
  /// in the 'surfaces' parameter list.
  RealVectorValue distanceVectorByIndex(unsigned int idx, const Point & pt) const;

  /// A true normal vector at the projected point of \p pt onto a specific surface specified
  /// by index \p idx in the 'surfaces' parameter list.
  RealVectorValue trueNormalByIndex(unsigned int idx, const Point & pt) const;

  /// Compute distance vector using a specific distance function
  RealVectorValue distanceVectorByFunc(const Point & pt, Real t, const Function * func) const;

  /// Compute true normal using a specific distance function
  RealVectorValue trueNormalByFunc(const Point & pt, Real t, const Function * func) const;

protected:
  /// Optional signed-distance function
  std::vector<const Function *> _distance_functions;
};
