//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"
#include "MooseVariableInterface.h"
#include "FaceArgInterface.h"

/**
 * This postprocessor computes a surface integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideIntegralVariablePostprocessor : public SideIntegralPostprocessor,
                                          public MooseVariableInterface<Real>,
                                          public FaceArgProducerInterface
{
public:
  static InputParameters validParams();

  SideIntegralVariablePostprocessor(const InputParameters & parameters);

  bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;

protected:
  Real computeQpIntegral() override;
  Real computeFaceInfoIntegral(const FaceInfo * fi) override;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;
  /// Whether this is acting on a finite volume variable
  const bool _fv;
};
