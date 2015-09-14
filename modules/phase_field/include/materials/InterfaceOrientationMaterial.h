/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERFACEORIENTATIONMATERIAL_H
#define INTERFACEORIENTATIONMATERIAL_H

#include "Material.h"

//Forward Declarations
class InterfaceOrientationMaterial;

template<>
InputParameters validParams<InterfaceOrientationMaterial>();

/**
 * Material to compute the angular orientation of order parameter interfaces.
 */
class InterfaceOrientationMaterial : public Material
{
public:
  InterfaceOrientationMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  MaterialProperty<Real> & _eps;
  MaterialProperty<Real> & _deps;

  VariableValue & _u;
  VariableGradient & _grad_u;
};

#endif //INTERFACEORIENTATIONMATERIAL_H
