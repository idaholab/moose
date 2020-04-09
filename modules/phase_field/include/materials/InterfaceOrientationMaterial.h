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
 * See R. Kobayashi, Physica D, 63, 410-423 (1993), final (non-numbered) equation
 * on p. 412. doi:10.1016/0167-2789(93)90120-P
 */
class InterfaceOrientationMaterial : public Material
{
public:
  static InputParameters validParams();

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
