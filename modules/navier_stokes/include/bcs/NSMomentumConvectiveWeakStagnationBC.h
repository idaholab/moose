/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMOMENTUMCONVECTIVEWEAKSTAGNATIONBC_H
#define NSMOMENTUMCONVECTIVEWEAKSTAGNATIONBC_H

#include "NSWeakStagnationBaseBC.h"

// Forward Declarations
class NSMomentumConvectiveWeakStagnationBC;

template <>
InputParameters validParams<NSMomentumConvectiveWeakStagnationBC>();

/**
 * The convective part (sans pressure term) of the momentum equation
 * boundary integral evaluated at specified stagnation temperature,
 * stagnation pressure, and flow direction values.
 */
class NSMomentumConvectiveWeakStagnationBC : public NSWeakStagnationBaseBC
{
public:
  NSMomentumConvectiveWeakStagnationBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  const unsigned int _component;
};

#endif // NSMOMENTUMCONVECTIVEWEAKSTAGNATIONBC_H
