//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVScalarLagrangeMultiplierConstraint.h"

#include "libmesh/enum_point_locator_type.h"

/**
 * This Kernel implements the residuals that enforce the constraint
 *
 * \phi(element E containing point P) = phi_0
 *
 * using a Lagrange multiplier approach. E.g. this kernel enforces the constraint that the
 * elemental value of \phi, in the element containing P, matches \phi_0
 *
 * In particular, this Kernel implements the residual contribution for
 * the lambda term in Eq. (5), and both terms in Eq. (6) where \int \phi_0 = V_0
 *
 * [0]: https://github.com/idaholab/large_media/blob/master/framework/scalar_constraint_kernel.pdf
 */
class FVPointValueConstraint : public FVScalarLagrangeMultiplierConstraint
{
public:
  static InputParameters validParams();

  FVPointValueConstraint(const InputParameters & parameters);
  virtual void meshChanged() override { setMyElem(); }

private:
  void setMyElem();
  ADReal computeQpResidual() override final;

  /// The point where the constraint should be enforced
  const Point _point;

  /// We use a point locator in case the constraint is a point value
  std::unique_ptr<libMesh::PointLocatorBase> _point_locator;

  /// Pointer to the element in case we have a point constraint
  const Elem * _my_elem;
};
