//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
