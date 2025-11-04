//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSBase.h"

/**
 * This calculates the time derivative for a coupled variable
 **/
class ConvectedMesh : public INSBase
{
public:
  static InputParameters validParams();

  ConvectedMesh(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /**
   * Compute the Jacobian of the Petrov-Galerkin stabilization addition with respect to the provided
   * velocity component
   */
  Real computePGVelocityJacobian(unsigned short component);

  /**
   * Compute the strong residual, e.g. -rho * ddisp/dt * grad(u)
   */
  Real strongResidual();

  const VariableValue & _disp_x_dot;
  const VariableValue & _d_disp_x_dot;
  const unsigned int _disp_x_id;
  const VariableValue & _disp_y_dot;
  const VariableValue & _d_disp_y_dot;
  const unsigned int _disp_y_id;
  const VariableValue & _disp_z_dot;
  const VariableValue & _d_disp_z_dot;
  const unsigned int _disp_z_id;

  const MaterialProperty<Real> & _rho;

  /// Whether this kernel should include Streamline-Upwind Petrov-Galerkin stabilization
  const bool _supg;

  /// The velocity component this kernel is acting on
  unsigned short _component;
};
