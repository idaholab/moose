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

class PointInSurfaceCheckInterface;

/**
 * Test-only aux kernel that exposes the raw tri-state classification of a
 * point-in-surface check user object: OUTSIDE -> 0, ON -> 1, INSIDE -> 2.
 *
 * The production spatialValue() collapses INSIDE and ON to 1, so it cannot show
 * the INSIDE > ON > OUTSIDE precedence of a union; this aux makes ON observable in
 * a gold file.
 */
class SurfaceSidenessTestAux : public AuxKernel
{
public:
  static InputParameters validParams();
  SurfaceSidenessTestAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Fetch the named user object and cast it to the interface, erroring on the
  /// user_object parameter if it does not implement it.
  const PointInSurfaceCheckInterface & getCheckedInterface();

  /// The point-in-surface check user object being observed.
  const PointInSurfaceCheckInterface & _check;
};
