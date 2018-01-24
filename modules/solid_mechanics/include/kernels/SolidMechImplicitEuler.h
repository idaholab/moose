//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SOLIDMECHIMPLICITEULER_H
#define SOLIDMECHIMPLICITEULER_H

#include "SecondDerivativeImplicitEuler.h"
#include "Material.h"

// Forward Declarations
class SolidMechImplicitEuler;

template <>
InputParameters validParams<SolidMechImplicitEuler>();

class SolidMechImplicitEuler : public SecondDerivativeImplicitEuler
{
public:
  SolidMechImplicitEuler(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real scaling();

private:
  const MaterialProperty<Real> & _density;
  const bool _artificial_scaling_set;
  const Real _artificial_scaling;
};
#endif // SOLIDMECHIMPLICITEULER_H
