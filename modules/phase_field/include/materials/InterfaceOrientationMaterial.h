/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERFACEORIENTATIONMATERIAL_H
#define INTERFACEORIENTATIONMATERIAL_H

#include "Material.h"

// Forward Declarations
class InterfaceOrientationMaterial;

template <>
InputParameters validParams<InterfaceOrientationMaterial>();

/**
 * Material to compute the angular orientation of order parameter interfaces.
 * See R. Kobayashi, Physica D, 63, 410-423 (1993), final (non-numbered) equation
 * on p. 412. doi:10.1016/0167-2789(93)90120-P
 */
class InterfaceOrientationMaterial : public Material
{
public:
  InterfaceOrientationMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  Real _delta;
  unsigned int _j;
  Real _theta0;
  Real _eps_bar;

  MaterialProperty<Real> & _eps;
  MaterialProperty<Real> & _deps;
  MaterialProperty<RealGradient> & _depsdgrad_op;
  MaterialProperty<RealGradient> & _ddepsdgrad_op;

  const VariableValue & _op;
  const VariableGradient & _grad_op;
};

#endif // INTERFACEORIENTATIONMATERIAL_H
