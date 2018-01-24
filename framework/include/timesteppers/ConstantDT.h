//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSTANTDT_H
#define CONSTANTDT_H

#include "TimeStepper.h"

class ConstantDT;

template <>
InputParameters validParams<ConstantDT>();

class ConstantDT : public TimeStepper
{
public:
  ConstantDT(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

private:
  const Real _constant_dt;
  const Real _growth_factor;
};

#endif /* CONSTANTDT_H */
