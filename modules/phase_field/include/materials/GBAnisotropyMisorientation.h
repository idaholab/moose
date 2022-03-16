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
 * Function[kappa, gamma, m, L] = parameters (sigma, mob, w_GB, sigma0)
 * Parameter determination method is elaborated in Phys. Rev. B, 78(2), 024113, 2008, by N. Moelans
 * This material uses Algorithm 1 from the paper to determine parameters for constant GB width
 */
class GBAnisotropyMisorientation : public Material
{
public:
  static InputParameters validParams();

  GBAnisotropyMisorientation(const InputParameters & parameters);

private:
  virtual void computeQpProperties();

  const unsigned int _mesh_dimension;

  const Real _wGB;
  const Real _length_scale;
  const Real _time_scale;
  const Real _M_V;

  const VariableValue & _T;

  Real _sigma; // 二维矩阵
  Real _mob;
  Real _Q;
  Real _kappa;
  Real _gamma;
  Real _a;
  Real _g2;

  MaterialProperty<Real> & _sigma_gb;
  MaterialProperty<Real> & _mob_gb;
  MaterialProperty<Real> & _kappa_gb;
  MaterialProperty<Real> & _gamma_gb;
  MaterialProperty<Real> & _L_gb;
  MaterialProperty<Real> & _mu_gb;

  MaterialProperty<Real> & _molar_volume;
  MaterialProperty<Real> & _entropy_diff;
  MaterialProperty<Real> & _act_wGB;

  const Real _kb;
  const Real _JtoeV;
  Real _mu_qp;

  const unsigned int _op_num;

  const std::vector<const VariableValue *> _vals;
  const std::vector<const VariableGradient *> _grad_vals;

  const MaterialProperty<Real> & _delta_theta;
};