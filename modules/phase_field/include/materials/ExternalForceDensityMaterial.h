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
class Function;

/**
 * This Material calculates the force density acting on a particle/grain
 * due to interaction between particles
 */
class ExternalForceDensityMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  ExternalForceDensityMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  const Function & _force_x;
  const Function & _force_y;
  const Function & _force_z;

  /// concentration field considered to be the density of particles
  const VariableValue & _c;
  VariableName _c_name;
  /// stiffness constant
  const Real _k;

  const unsigned int _op_num;
  const std::vector<const VariableValue *> _vals;
  const std::vector<VariableName> _vals_name;

  /// force density material
  MaterialProperty<std::vector<RealGradient>> & _dF;
  /// first order derivative of force density material w.r.t c
  MaterialProperty<std::vector<RealGradient>> * _dFdc;

  std::vector<MaterialProperty<std::vector<RealGradient>> *> _dFdeta;
};
