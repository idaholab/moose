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

// Forward Declarations

/**
 * Material to compute the angular orientation of order parameter interfaces.
 */
class InterfaceOrientationMultiphaseMaterial : public Material
{
public:
  static InputParameters validParams();

  InterfaceOrientationMultiphaseMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  MaterialPropertyName _kappa_name;
  MaterialPropertyName _dkappadgrad_etaa_name;
  MaterialPropertyName _d2kappadgrad_etaa_name;
  Real _delta;
  unsigned int _j;
  Real _theta0;
  Real _kappa_bar;

  MaterialProperty<Real> & _kappa;
  MaterialProperty<RealGradient> & _dkappadgrad_etaa;
  MaterialProperty<RealTensorValue> & _d2kappadgrad_etaa;

  const VariableValue & _etaa;
  const VariableGradient & _grad_etaa;

  const VariableValue & _etab;
  const VariableGradient & _grad_etab;
};
