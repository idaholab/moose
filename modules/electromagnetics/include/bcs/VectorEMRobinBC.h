//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorIntegratedBC.h"
#include <complex>

/**
 *  First order Robin-style Absorbing/Port boundary condition for vector nonlinear
 *  variables.
 */
class VectorEMRobinBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorEMRobinBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Scalar waveguide propagation constant
  const Function & _beta;

  /// Variable field component (real or imaginary)
  const MooseEnum _component;

  /// Coupled field vector variable
  const VectorVariableValue & _coupled_val;

  /// Coupled field vector variable id
  unsigned int _coupled_var_num;

  /// Real incoming field vector
  const Function & _inc_real;

  /// Imaginary incoming field vector
  const Function & _inc_imag;

  /// Mode of operation (absorbing or port)
  const MooseEnum _mode;

  /// Boolean marking whether real component of the incoming wave was set by the user
  const bool _real_incoming_was_set;

  /// Boolean marking whether imaginary component of the incoming wave was set by the user
  const bool _imag_incoming_was_set;
};
