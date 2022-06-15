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
 *  First order transient absorbing boundary condition for nonlinear vector variables
 */
class VectorTransientAbsorbingBC : public VectorIntegratedBC
{
public:
  static InputParameters validParams();

  VectorTransientAbsorbingBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Intrinsic impedance of the infinite medium (default is the admittance of free space)
  const Function & _admittance;

  /// Magnetic permeability of free space in SI units (H/m)
  const Real _mu0;

  /// Variable field component (real or imaginary)
  const MooseEnum _component;

  /// Coupled field vector variable
  const VectorVariableValue & _coupled_val;
  /// Coupled field vector variable id
  const unsigned int _coupled_var_num;

  /// Vector field variable time derivative
  const VectorVariableValue & _u_dot;

  /// Coupled vector field variable time derivative
  const VectorVariableValue & _coupled_dot;

  /// Vector field variable dot du
  const VariableValue & _du_dot_du;

  /// Coupled vector field variable dot du
  const VariableValue & _coupled_dot_du;
};
