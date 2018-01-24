//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ANISOHEATCONDUCTION_H
#define ANISOHEATCONDUCTION_H

#include "Kernel.h"

class AnisoHeatConduction : public Kernel
{
public:
  AnisoHeatConduction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  const unsigned _dim;

  const MaterialProperty<Real> * _k_i[3];
  const MaterialProperty<Real> * _k_i_dT[3];
};

template <>
InputParameters validParams<AnisoHeatConduction>();

#endif // ANISOHEATCONDUCTION_H
