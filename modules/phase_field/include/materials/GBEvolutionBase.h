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

template <bool is_ad>
class GBEvolutionBaseTempl : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  GBEvolutionBaseTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  Real _f0s;
  Real _wGB;
  Real _length_scale;
  Real _time_scale;
  Real _GBmob0;
  Real _Q;
  Real _GBMobility;
  Real _molar_vol;

  const VariableValue & _T;

  GenericMaterialProperty<Real, is_ad> & _sigma;
  GenericMaterialProperty<Real, is_ad> & _M_GB;
  GenericMaterialProperty<Real, is_ad> & _kappa;
  GenericMaterialProperty<Real, is_ad> & _gamma;
  GenericMaterialProperty<Real, is_ad> & _L;
  MaterialProperty<Real> * _dLdT;
  GenericMaterialProperty<Real, is_ad> & _l_GB;
  GenericMaterialProperty<Real, is_ad> & _mu;
  GenericMaterialProperty<Real, is_ad> & _entropy_diff;
  GenericMaterialProperty<Real, is_ad> & _molar_volume;
  GenericMaterialProperty<Real, is_ad> & _act_wGB;

  const Real _kb;
  const Real _JtoeV;
};

typedef GBEvolutionBaseTempl<false> GBEvolutionBase;
typedef GBEvolutionBaseTempl<true> ADGBEvolutionBase;
