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

#ifndef MEMOIZEFUNCTIONINTERFACE_H
#define MEMOIZEFUNCTIONINTERFACE_H

// C++ includes
#include <map>

// MOOSE includes
#include "Function.h"

// Module includes
#include "Hashing.h"

// Forward declarations
class MemoizedFunctionInterface;

template <>
InputParameters validParams<MemoizedFunctionInterface>();

/**
 * Implementation of Function that memoizes (caches) former evaluations in an unordered map using a
 * hash of the evaluation locations as the key. The purpose is to allow for quick evaluation of a
 * complex function that may be reevaluated multiple times without changing the actual outputs.
 */
class MemoizedFunctionInterface : public Function
{
public:
  MemoizedFunctionInterface(const InputParameters & parameters);

  // Override from MeshChangedInterface
  virtual void meshChanged() override;

  /// Enable/disable the cache
  void useCache(bool use);

  /*
   * Make this implementation of Function::Value() final so derived classes cannot bypass the
   * memoization functionality it implements. Instead, deriving classes should implement
   * evaluateValue().
   */
  virtual Real value(Real time, const Point & point) final;

protected:
  /// Used in derived classes, equivalent to Function::value()
  virtual Real evaluateValue(Real time, const Point & point) = 0;

  /// Called by derived classes to invalidate the cache, perhaps due to a state change
  void invalidateCache();

private:
  /// Cached evaluations for each point
  std::unordered_map<hashing::HashValue, Real> _cache;

  /// Stores the time evaluation of the cache
  Real _current_time;

  /// Flag for whether to cache values
  bool _enable_cache;

  /// Flag for whether changes in time invalidate the cache
  bool _respect_time;
};

#endif // MEMOIZEFUNCTIONINTERFACE_H
