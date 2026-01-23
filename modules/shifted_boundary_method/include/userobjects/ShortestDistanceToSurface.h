//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "ElementUserObject.h"

/**
 * Base distance user object that manages function/mesh-based distance strategies.
 * Derived classes can specialize how the strategies are queried or cached.
 */
class ShortestDistanceToSurface : public ElementUserObject
{
public:
  static InputParameters validParams();
  ShortestDistanceToSurface(const InputParameters & parameters);
  void execute() override {};
  void finalize() override {}
  void initialize() override {}
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
  const RealVectorValue distanceVectorByFunc(const Point & pt, Real t, const Function * func) const;

  /// Compute true normal using a specific distance function
  const RealVectorValue trueNormalByFunc(const Point & pt, Real t, const Function * func) const;

protected:
  /// Optional signed-distance function
  std::vector<const Function *> _distance_functions;
};
