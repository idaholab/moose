//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <map>

#include "Function.h"

#include "Hashing.h"

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

  /**
   * Enable/disable the cache
   */
  void useCache(bool use);

  // Make this implementation of Function::value() final so derived classes cannot bypass the
  // memoization functionality it implements. Instead, deriving classes should implement
  // evaluateValue().
  virtual Real value(Real time, const Point & point) final;

protected:
  /**
   * Used in derived classes, equivalent to Function::value()
   */
  virtual Real evaluateValue(Real time, const Point & point) = 0;

  /**
   * Called by derived classes to invalidate the cache, perhaps due to a state change
   */
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

