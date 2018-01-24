/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENERGYINVISCIDSPECIFIEDDENSITYANDVELOCITYBC_H
#define NSENERGYINVISCIDSPECIFIEDDENSITYANDVELOCITYBC_H

#include "NSEnergyInviscidBC.h"

// Forward Declarations
class NSEnergyInviscidSpecifiedDensityAndVelocityBC;

template <>
InputParameters validParams<NSEnergyInviscidSpecifiedDensityAndVelocityBC>();

/**
 * The inviscid energy BC term with specified density and velocity components.
 * This was experimental code and did not really work out, do not use!
 */
class NSEnergyInviscidSpecifiedDensityAndVelocityBC : public NSEnergyInviscidBC
{
public:
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

#endif // NSENERGYINVISCIDSPECIFIEDDENSITYANDVELOCITYBC_H
