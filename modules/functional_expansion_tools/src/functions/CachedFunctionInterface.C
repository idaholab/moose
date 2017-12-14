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

// Module includes
#include "CachedFunctionInterface.h"

template <>
InputParameters
validParams<CachedFunctionInterface>()
{
  InputParameters params = validParams<Function>();

  params.addClassDescription("The function uses a cache to potentially reduce"
                             " the computational burden of reusing a complex or"
                             " costly function");

  params.addParam<bool>("enable_cache",
                        false,
                        "Enables cached function evaluations. Recommended only"
                        " if this function is used directly in a BC or Kernel."
                        " This will be enabled automatically if any of the"
                        " FE-based BCs are used.");

  params.addParam<bool>("respect_time", false, "Enable to clear the cache at each new time step.");

  return params;
}

CachedFunctionInterface::CachedFunctionInterface(const InputParameters & parameters)
  : Function(parameters),
    _enable_cache(getParam<bool>("enable_cache")),
    _respect_time(getParam<bool>("respect_time"))
{
  // Nothing here
}

void
CachedFunctionInterface::meshChanged()
{
  // The mesh has changed, which invalidates the cache
  invalidateCache();
}

Real
CachedFunctionInterface::value(Real time, const Point & point)
{
  if (_enable_cache)
  {
    // Start the cache over if we are at a new time step
    if (_respect_time && time != _current_time)
      _current_time = time, invalidateCache();

    // Try to insert a new value into the cache
    auto result = _cache.insert({hashing::hashCombine(time, point), 0.0});

    // Evaluate and apply if the insertion worked, i.e. the element didn't exist
    if (result.second)
      result.first->second = evaluateValue(time, point);

    // Return the cached value
    return result.first->second;
  }

  return evaluateValue(time, point);
}

void
CachedFunctionInterface::useCache(bool use)
{
  _enable_cache = use;

  if (!_enable_cache)
    invalidateCache();
}

void
CachedFunctionInterface::invalidateCache()
{
  _cache.clear();
}
