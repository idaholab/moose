//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"
#include "MooseTypes.h"

// Forward Declarations
class Function;

/**
 * Simple material with diffusivity (D) defined using an Arrhenius relation.
 */
class ADArrheniusMaterial : public ADMaterial
{
public:
  static InputParameters validParams();

  ADArrheniusMaterial(const InputParameters & parameters);

protected:
  virtual void computeProperties();

  const ADVariableValue & _ad_temperature;

  const Real _my_pre_exponential;
  const Real _my_activation_energy;
  const Real _my_ideal_gas_constant;
  const MaterialPropertyName _diffusivity_name;
  ADMaterialProperty<Real> & _pre_exponential;

  ADMaterialProperty<Real> & _activation_energy;
  ADMaterialProperty<Real> & _ideal_gas_constant;
  ADMaterialProperty<Real> & _diffusivity;

private:
  void setDerivatives(ADReal & prop, Real dprop_dT, const ADReal & ad_T);
};

