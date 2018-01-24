//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALTIMESTEPPOSTPROCESSOR_H
#define MATERIALTIMESTEPPOSTPROCESSOR_H

#include "ElementPostprocessor.h"

class MaterialTimeStepPostprocessor;

template <>
InputParameters validParams<MaterialTimeStepPostprocessor>();

/**
 * This postporocessor calculates an estimated timestep size that limits
 * an auxiliary variable to below a given threshold.
*/
class MaterialTimeStepPostprocessor : public ElementPostprocessor
{
public:
  MaterialTimeStepPostprocessor(const InputParameters & parameters);
  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  const MaterialProperty<Real> & _matl_time_step;

  Real _value;
  unsigned int _qp;
};

#endif // MATERIALTIMESTEPPOSTPROCESSOR_H
