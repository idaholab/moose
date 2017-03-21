/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GBANISOTROPYBASE_H
#define GBANISOTROPYBASE_H

#include "Material.h"

// Forward Declarations
class GBAnisotropyBase;

template <>
InputParameters validParams<GBAnisotropyBase>();

/**
 * Function[kappa, gamma, m, L] = parameters (sigma, mob, w_GB, sigma0)
 * Parameter determination method is elaborated in Phys. Rev. B, 78(2), 024113, 2008, by N. Moelans
 * Thanks to Prof. Moelans for the explanation of her paper.
 */
class GBAnisotropyBase : public Material
{
public:
  GBAnisotropyBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const unsigned int _mesh_dimension;

  const Real _length_scale;
  const Real _time_scale;
  const Real _M_V;
  const Real _delta_sigma;
  const Real _delta_mob;

  const FileName _Anisotropic_GB_file_name;

  const bool _inclination_anisotropy;

  const VariableValue & _T;

  std::vector<std::vector<Real>> _sigma;
  std::vector<std::vector<Real>> _mob;
  std::vector<std::vector<Real>> _Q;
  std::vector<std::vector<Real>> _kappa_gamma;
  std::vector<std::vector<Real>> _a_g2;

  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _gamma;
  MaterialProperty<Real> & _L;
  MaterialProperty<Real> & _mu;

  MaterialProperty<Real> & _molar_volume;
  MaterialProperty<Real> & _entropy_diff;
  MaterialProperty<Real> & _act_wGB;
  MaterialProperty<Real> & _tgrad_corr_mult;

  const Real _kb;
  const Real _JtoeV;
  Real _mu_qp;

  const unsigned int _op_num;

  std::vector<const VariableValue *> _vals;
  std::vector<const VariableGradient *> _grad_vals;
};

#endif // GBANISOTROPYBASE_H
