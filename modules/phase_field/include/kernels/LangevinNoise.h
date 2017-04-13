/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
