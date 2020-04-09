//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"

// Forward Declarations
class Function;

/**
 * This kernel implements a generic functional
 * body force term:
 * $ - c \cdof f \cdot \phi_i $
 *
 * The coefficient and function both have defaults
 * equal to 1.0.
 */
class InterfacialSource : public InterfaceKernel
{
public:
  static InputParameters validParams();

  InterfacialSource(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  void computeElemNeighResidual(Moose::DGResidualType type) override;
  void computeElemNeighJacobian(Moose::DGJacobianType type) override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  const Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;

  const MooseArray<Real> & _neighbor_JxW;
};
