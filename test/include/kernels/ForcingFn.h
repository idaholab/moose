//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FORCINGFN_H_
#define FORCINGFN_H_

#include "Kernel.h"

class ForcingFn : public Kernel
{
public:
  ForcingFn(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real funcValue();
};

template <>
InputParameters validParams<ForcingFn>();

#endif /* FORCINGFN_H_ */
