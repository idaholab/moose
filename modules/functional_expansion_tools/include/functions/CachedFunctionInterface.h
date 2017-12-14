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

#ifndef CACHEDFUNCTIONINTERFACE_H
#define CACHEDFUNCTIONINTERFACE_H

// C++ includes
#include <map>

// MOOSE includes
#include "Function.h"

// Module includes
#include "Hashing.h"

// Forward declarations
class CachedFunctionInterface;

template <>
InputParameters validParams<CachedFunctionInterface>();

/**
 * Implementation of Function that caches evaluations in an unordered map using
 * a hash of the evaluation locations as the key. The purpose is to allow for
 * quick evaluation of a complex function that may be reevaluated multiple times
 * without changing the actual outputs.
 */
class CachedFunctionInterface : public Function
{
public:
  CachedFunctionInterface(const InputParameters & parameters);

  /// Overriden member from MeshChangedInterface
  virtual void meshChanged();

  /// Replaces Function::value in derived classes
  virtual Real evaluateValue(Real time, const Point & point) = 0;

  /// Use the cache
  void useCache(bool use);

  /// Make this final so all derived classes cannot bypass the cache functionality
  virtual Real value(Real time, const Point & point) final;

protected:
  /**
   * Called by derived classes to invalidate the cache, perhaps due to a state
   * change
   */
  void invalidateCache();

private:
  /// Cached evaluations for each point
  std::unordered_map<hashing::HashValue, Real> _cache;

  /// Flag whether to cache values
  bool _enable_cache;

  /// Flag whether changes in time invalidate the cache
  bool _respect_time;

  /// Stores the time evaluation of the cache
  Real _current_time;
};

#endif // CACHEDFUNCTIONINTERFACE_H
