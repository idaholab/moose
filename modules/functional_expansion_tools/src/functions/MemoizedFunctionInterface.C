//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MemoizedFunctionInterface.h"

InputParameters
MemoizedFunctionInterface::validParams()
{
  InputParameters params = Function::validParams();

  params.addClassDescription("The function uses a cache to potentially reduce the computational "
                             "burden of reusing a complex or costly function");

  params.addParam<bool>("enable_cache",
                        false,
                        "Enables cached function evaluations. Recommended only if this function is "
                        "used directly in a BC or Kernel. This will be enabled automatically if "
                        "any of the FX-based BCs are used.");

  params.addParam<bool>("respect_time", false, "Enable to clear the cache at each new time step.");

  return params;
}

MemoizedFunctionInterface::MemoizedFunctionInterface(const InputParameters & parameters)
  : Function(parameters),
    _enable_cache(getParam<bool>("enable_cache")),
    _respect_time(getParam<bool>("respect_time"))
{
}

void
MemoizedFunctionInterface::meshChanged()
{
  // The mesh has changed, which invalidates the cache
  invalidateCache();
}

Real
MemoizedFunctionInterface::value(Real time, const Point & point) const
{
  MemoizedFunctionInterface * ptr = const_cast<MemoizedFunctionInterface *>(this);

  if (_enable_cache)
  {
    // Start the cache over if we are at a new time step
    if (_respect_time && time != _current_time)
    {
      _current_time = time;
      ptr->invalidateCache();
    }

    // Try to insert a new value into the cache
    auto result = _cache.insert({hashing::hashCombine(time, point), 0.0});

    // Evaluate and apply if the insertion worked, i.e. the element didn't exist
    if (result.second)
      result.first->second = ptr->evaluateValue(time, point);

    // Return the cached value
    return result.first->second;
  }

  return ptr->evaluateValue(time, point);
}

ADReal
MemoizedFunctionInterface::value(const ADReal &, const ADPoint &) const
{
  mooseError("Not implemented");
}

void
MemoizedFunctionInterface::useCache(bool use)
{
  _enable_cache = use;

  if (!_enable_cache)
    invalidateCache();
}

void
MemoizedFunctionInterface::invalidateCache()
{
  _cache.clear();
}
