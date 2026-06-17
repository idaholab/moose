//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class ShortestDistanceToSurface;
class Function;

/**
 * Test-only AuxKernel that exposes the rarely-called ShortestDistanceToSurface
 * accessors (trueNormal, *ByIndex, *ByFunc) so coverage tests can exercise them
 * without adding production callers.
 */
class ShortestDistanceToSurfaceTestAux : public AuxKernel
{
public:
  static InputParameters validParams();

  explicit ShortestDistanceToSurfaceTestAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  enum class Method
  {
    TRUE_NORMAL,
    DISTANCE_BY_INDEX,
    TRUE_NORMAL_BY_INDEX,
    DISTANCE_BY_FUNC,
    TRUE_NORMAL_BY_FUNC,
  };

  enum class Component
  {
    X,
    Y,
    Z,
    NORM,
  };

  const ShortestDistanceToSurface & _distance_to_surface;
  const Method _method;
  const Component _component;
  const unsigned int _index;
  const Function * const _function;
};
