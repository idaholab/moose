//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TIMEDERIVATIVE_H
#define TIMEDERIVATIVE_H

#include "TimeKernel.h"

// Forward Declaration
class TimeDerivative;

template <>
InputParameters validParams<TimeDerivative>();

class TimeDerivative : public TimeKernel
{
public:
  TimeDerivative(const InputParameters & parameters);

  virtual void computeJacobian() override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  bool _lumping;
};

#endif // TIMEDERIVATIVE_H
