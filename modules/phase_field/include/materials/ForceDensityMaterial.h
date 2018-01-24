/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FORCEDENSITYMATERIAL_H
#define FORCEDENSITYMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class ForceDensityMaterial;

template <>
InputParameters validParams<ForceDensityMaterial>();

/**
 * This Material calculates the force density acting on a particle/grain
 * due to interaction between particles
 */
class ForceDensityMaterial : public DerivativeMaterialInterface<Material>
{
public:
  ForceDensityMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  /// concentration field considered to be the density of particles
  const VariableValue & _c;
  VariableName _c_name;
  /// equilibrium density at the grain boundaries
  Real _ceq;
  /// thresold value for identifying grain boundaries
  Real _cgb;
  /// stiffness constant
  Real _k;

  unsigned int _op_num;
  std::vector<const VariableValue *> _vals;
  std::vector<const VariableGradient *> _grad_vals;
  std::vector<VariableName> _vals_name;

  std::vector<Real> _product_etas;
  std::vector<RealGradient> _sum_grad_etas;

  /// type of force density material
  std::string _base_name;

  /// force density material
  MaterialProperty<std::vector<RealGradient>> & _dF;
  /// first order derivative of force density material w.r.t c
  MaterialProperty<std::vector<RealGradient>> & _dFdc;
  /// first order derivative of force density material w.r.t etas
  std::vector<MaterialProperty<std::vector<Real>> *> _dFdgradeta;
};

#endif // FORCEDENSITYMATERIAL_H
