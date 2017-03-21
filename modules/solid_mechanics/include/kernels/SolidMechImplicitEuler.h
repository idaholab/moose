/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
