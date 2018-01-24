//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXAMPLETIMEDERIVATIVE
#define EXAMPLETIMEDERIVATIVE

#include "TimeDerivative.h"

// Forward Declarations
class ExampleTimeDerivative;

template <>
InputParameters validParams<ExampleTimeDerivative>();

class ExampleTimeDerivative : public TimeDerivative
{
public:
  ExampleTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  Real _time_coefficient;
};

#endif // EXAMPLETIMEDERIVATIVE
