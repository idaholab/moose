//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * This Material calculates the force density acting on a particle/grain
 * due to interaction between particles
 */
class ForceDensityMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

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

  const unsigned int _op_num;
  const std::vector<const VariableValue *> _vals;
  const std::vector<const VariableGradient *> _grad_vals;
  const std::vector<VariableName> _vals_name;

  std::vector<Real> _product_etas;
  std::vector<RealGradient> _sum_grad_etas;

  /// type of force density material
  const std::string _base_name;

  /// force density material
  MaterialProperty<std::vector<RealGradient>> & _dF;
  /// first order derivative of force density material w.r.t c
  MaterialProperty<std::vector<RealGradient>> & _dFdc;
  /// first order derivative of force density material w.r.t etas
  std::vector<MaterialProperty<std::vector<Real>> *> _dFdgradeta;
};
