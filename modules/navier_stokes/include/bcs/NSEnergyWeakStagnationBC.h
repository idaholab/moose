/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENERGYWEAKSTAGNATIONBC_H
#define NSENERGYWEAKSTAGNATIONBC_H

#include "NSWeakStagnationBaseBC.h"

// Forward Declarations
class NSEnergyWeakStagnationBC;

template <>
InputParameters validParams<NSEnergyWeakStagnationBC>();

/**
 * The inviscid energy BC term with specified normal flow.
 */
class NSEnergyWeakStagnationBC : public NSWeakStagnationBaseBC
{
public:
  NSEnergyWeakStagnationBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif // NSENERGYWEAKSTAGNATIONBC_H
