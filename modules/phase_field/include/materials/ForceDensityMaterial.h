/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FORCEDENSITYMATERIAL_H
#define FORCEDENSITYMATERIAL_H

#include "Material.h"

//Forward Declarations
class ForceDensityMaterial;

template<>
InputParameters validParams<ForceDensityMaterial>();

/**
 * This Material calculates the force density acting on a particle/grain
 * due to interaction between particles
 */
class ForceDensityMaterial : public Material
{
public:
  ForceDensityMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  /// concentration field considered to be the density of particles
  VariableValue & _c;
  /// equilibrium density at the grain boundaries
  Real _ceq;
  /// thresold value for identifying grain boundaries
  Real _cgb;
  /// stiffness constant
  Real _k;

  unsigned int _ncrys;
  std::vector<VariableValue *> _vals;
  std::vector<VariableGradient *> _grad_vals;

  std::vector<Real> _product_etas;
  std::vector<RealGradient> _sum_grad_etas;

  /// force density material
  MaterialProperty<std::vector<RealGradient> > & _dF;
  /// first order derivative of force density material w.r.t c
  MaterialProperty<std::vector<RealGradient> > & _dFdc;
};

#endif //FORCEDENSITYMATERIAL_H
