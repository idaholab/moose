/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMASSWEAKSTAGNATIONBC_H
#define NSMASSWEAKSTAGNATIONBC_H

#include "NSWeakStagnationBaseBC.h"

// Forward Declarations
class NSMassWeakStagnationBC;

template <>
InputParameters validParams<NSMassWeakStagnationBC>();

/**
 * The inviscid energy BC term with specified normal flow.
 */
class NSMassWeakStagnationBC : public NSWeakStagnationBaseBC
{
public:
  NSMassWeakStagnationBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif // NSMASSWEAKSTAGNATIONBC_H
