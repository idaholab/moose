//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"
#include "MooseVariableInterface.h"
#include "BoundaryShortestDistanceToSurface.h"

/**
 * This postprocessor computes a surface integral of the shifted specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideIntegralShiftedVariablePostprocessor : public SideIntegralPostprocessor,
                                                 public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  SideIntegralShiftedVariablePostprocessor(const InputParameters & parameters);

protected:
  Real computeQpIntegral() override;

  virtual void initialSetup() override final;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// Distance and normal provider userobject
  const BoundaryShortestDistanceToSurface * _sbm_distance_uo = nullptr;

  /// @brief Flag indicating whether shifted terms are used
  bool _shifted;

  /// @brief Flag indicating whether the shifted variable (u + grad_u * d) is used
  bool _shifted_variable;
};
