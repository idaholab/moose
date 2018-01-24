//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef DEFAULTMATPROPCONSUMERKERNEL_H
#define DEFAULTMATPROPCONSUMERKERNEL_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

// Forward declarations
class DefaultMatPropConsumerKernel;

template <>
InputParameters validParams<DefaultMatPropConsumerKernel>();

class DefaultMatPropConsumerKernel : public DerivativeMaterialInterface<Kernel>
{
public:
  DefaultMatPropConsumerKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() { return 0.0; };

  const MaterialProperty<Real> & _prop;
};

#endif // DEFAULTMATPROPCONSUMERKERNEL_H
