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
class SBMSurfaceMeshBuilder;
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

  /// If a 'builder' was supplied, calls SBMSurfaceMeshBuilder::getCentroids() once
  /// and verifies it is non-empty. Used purely so coverage tests can touch the
  /// otherwise-unused getter from production code paths.
  virtual void initialSetup() override;

protected:
  virtual Real computeValue() override;

private:
  const ShortestDistanceToSurface & _distance_to_surface;
  const SBMSurfaceMeshBuilder * const _builder;
  const MooseEnum _method;
  const MooseEnum _component;
  const unsigned int _index;
  const Function * const _function;
  /// If set, queries are evaluated at this fixed point instead of the
  /// current element centroid. Used to drive accessors at specific locations
  /// (e.g. on a surface-mesh node to exercise zero-distance branches).
  const bool _use_at_point;
  const Point _at_point;
};
