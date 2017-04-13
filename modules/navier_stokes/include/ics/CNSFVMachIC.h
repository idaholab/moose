/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVMACHIC_H
#define CNSFVMACHIC_H

#include "InitialCondition.h"
#include "SinglePhaseFluidProperties.h"

class CNSFVMachIC;

template <>
InputParameters validParams<CNSFVMachIC>();

/**
 * An initial condition object for computing Mach number from conserved variables
 */
class CNSFVMachIC : public InitialCondition
{
public:
  CNSFVMachIC(const InputParameters & parameters);
  virtual ~CNSFVMachIC();

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
