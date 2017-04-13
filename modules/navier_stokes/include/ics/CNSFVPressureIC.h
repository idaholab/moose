/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVPRESSUREIC_H
#define CNSFVPRESSUREIC_H

#include "InitialCondition.h"
#include "SinglePhaseFluidProperties.h"

class CNSFVPressureIC;

template <>
InputParameters validParams<CNSFVPressureIC>();

/**
 * An initial condition object for computing pressure from conserved variables
 */
class CNSFVPressureIC : public InitialCondition
{
public:
  CNSFVPressureIC(const InputParameters & parameters);
  virtual ~CNSFVPressureIC();

  virtual Real value(const Point & p);

protected:
  const VariableValue & _rho;
  const VariableValue & _rhou;
  const VariableValue & _rhov;
  const VariableValue & _rhow;
  const VariableValue & _rhoe;

  const SinglePhaseFluidProperties & _fp;
};

#endif
