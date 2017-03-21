/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GBEVOLUTIONBASE_H
#define GBEVOLUTIONBASE_H

#include "Material.h"

// Forward Declarations
class GBEvolutionBase;

template <>
InputParameters validParams<GBEvolutionBase>();

class GBEvolutionBase : public Material
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
  MaterialProperty<Real> & _l_GB;
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _entropy_diff;
  MaterialProperty<Real> & _molar_volume;
  MaterialProperty<Real> & _act_wGB;
  MaterialProperty<Real> & _tgrad_corr_mult;

  const Real _kb;
  const Real _JtoeV;
};

#endif // GBEVOLUTIONBASE_H
