//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GBEVOLUTIONBASE_H
#define GBEVOLUTIONBASE_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class GBEvolutionBase;

template <>
InputParameters validParams<GBEvolutionBase>();

class GBEvolutionBase : public DerivativeMaterialInterface<Material>
{
public:
  GBEvolutionBase(const InputParameters & parameters);

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

  MaterialProperty<Real> & _sigma;
  MaterialProperty<Real> & _M_GB;
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _gamma;
  MaterialProperty<Real> & _L;
  MaterialProperty<Real> * _dLdT;
  MaterialProperty<Real> & _l_GB;
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _entropy_diff;
  MaterialProperty<Real> & _molar_volume;
  MaterialProperty<Real> & _act_wGB;

  const Real _kb;
  const Real _JtoeV;
};

#endif // GBEVOLUTIONBASE_H
