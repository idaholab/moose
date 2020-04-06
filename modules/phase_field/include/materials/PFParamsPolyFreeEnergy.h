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
 * Calculated properties for a single component phase field model using polynomial free energies
 */
class PFParamsPolyFreeEnergy : public Material
{
public:
  static InputParameters validParams();

  PFParamsPolyFreeEnergy(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  ///Variable values
  const VariableValue & _c;
  const VariableValue & _T;

  ///Mateiral property declarations
  MaterialProperty<Real> & _M;
  MaterialProperty<RealGradient> & _grad_M;

  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _c_eq;
  MaterialProperty<Real> & _W;
  MaterialProperty<Real> & _Qstar;
  MaterialProperty<Real> & _D;

  ///Input parameters
  Real _int_width;
  Real _length_scale;
  Real _time_scale;
  MooseEnum _order;
  Real _D0;
  Real _Em;
  Real _Ef;
  Real _surface_energy;

  const Real _JtoeV;
  const Real _kb;
};
