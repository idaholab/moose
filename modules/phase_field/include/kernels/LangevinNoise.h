//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LANGEVINNOISE_H
#define LANGEVINNOISE_H

#include "Kernel.h"

// Forward Declarations
class LangevinNoise;

template <>
InputParameters validParams<LangevinNoise>();

class LangevinNoise : public Kernel
{
public:
  LangevinNoise(const InputParameters & parameters);

protected:
  virtual void residualSetup();
  virtual Real computeQpResidual();

  const Real _amplitude;
  const MaterialProperty<Real> & _multiplier_prop;
};

#endif // LANGEVINNOISE_H
