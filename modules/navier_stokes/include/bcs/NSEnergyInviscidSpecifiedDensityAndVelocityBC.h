//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSEnergyInviscidBC.h"

// Forward Declarations

/**
 * The inviscid energy BC term with specified density and velocity components.
 * This was experimental code and did not really work out, do not use!
 */
class NSEnergyInviscidSpecifiedDensityAndVelocityBC : public NSEnergyInviscidBC
{
public:
  static InputParameters validParams();

  NSEnergyInviscidSpecifiedDensityAndVelocityBC(const InputParameters & parameters);

  virtual ~NSEnergyInviscidSpecifiedDensityAndVelocityBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Aux Variables
  const VariableValue & _pressure;

  // Required parameters
  Real _specified_density;

  Real _specified_u; // FIXME: Read these as a single RealVectorValue
  Real _specified_v; // FIXME: Read these as a single RealVectorValue
  Real _specified_w; // FIXME: Read these as a single RealVectorValue
};
