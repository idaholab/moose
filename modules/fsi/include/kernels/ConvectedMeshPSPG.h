//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSBase.h"

/**
 *  Computes residual and Jacobian contributions for the PSPG stabilization term for mesh advection
 **/
class ConvectedMeshPSPG : public INSBase
{
public:
  static InputParameters validParams();

  ConvectedMeshPSPG(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  RealVectorValue dStrongResidualDDisp(unsigned short component);
  RealVectorValue dStrongResidualDVel(unsigned short component);

  /**
   * Compute the strong residual, e.g. -rho * ddisp/dt * grad(u)
   */
  RealVectorValue strongResidual();

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
};
