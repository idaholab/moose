//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * Boundary hydrostatic correction for SWE momentum equations (wall analogue).
 *
 * Adds -0.5*g*h^2*n_component to the momentum residual at the boundary. When used together
 * with a wall flux that includes +0.5*g*h^2*n_component in the momentum flux, the two cancel
 * in lake-at-rest, preserving a flat free surface.
 */
class SWEHydrostaticCorrectionBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  SWEHydrostaticCorrectionBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // face value of depth
  const MaterialProperty<Real> & _h;

  // coupled variables indices
  const unsigned int _h_var;

  // gravity (coupled scalar field)
  const VariableValue & _g;
};
