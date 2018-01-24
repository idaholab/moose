//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ODETIMEDERIVATIVE_H
#define ODETIMEDERIVATIVE_H

#include "ODETimeKernel.h"

// Forward Declaration
class ODETimeDerivative;

template <>
InputParameters validParams<ODETimeDerivative>();

class ODETimeDerivative : public ODETimeKernel
{
public:
  ODETimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
};

#endif // ODETIMEDERIVATIVE_H
