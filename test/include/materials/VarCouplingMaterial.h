/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#ifndef VARCOUPLINGMATERIAL_H_
#define VARCOUPLINGMATERIAL_H_

#include "Material.h"

class VarCouplingMaterial;

template <>
InputParameters validParams<VarCouplingMaterial>();

/**
 * A material that couples a variable
 */
class VarCouplingMaterial : public Material
{
public:
  VarCouplingMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();

  const VariableValue & _var;
  Real _base;
  Real _coef;
  MaterialProperty<Real> & _diffusion;
  const MaterialProperty<Real> * const _diffusion_old;
};

#endif // VARCOUPLINGMATERIAL_H
