/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SIMPLESPLITCHWRES_H
#define SIMPLESPLITCHWRES_H

#include "Kernel.h"

class SimpleSplitCHWRes;

template <>
InputParameters validParams<SimpleSplitCHWRes>();

/**
 * Simple case for SplitCHWRes kernel, only with constant Mobility
 */
class SimpleSplitCHWRes : public Kernel
{
public:
  SimpleSplitCHWRes(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// Mobility
  const MaterialProperty<Real> & _M;
};

#endif // SIMPLESPLITCHWRES_H
