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

/**
 * A material that computes 304L volumetric stainless steel properties relevant to doing laser
 * welding modeling. The functional form of these properties come from:
 *
 * techreport{noble2007use,
 *   title={Use of Aria to simulate laser weld pool dynamics for neutron generator production.},
 *   author={Noble, David R and Notz, Patrick K and Martinez, Mario J and Kraynik, Andrew Michael},
 *   year={2007},
 *   institution={Sandia National Laboratories (SNL), Albuquerque, NM, and Livermore, CA}}
 */
class AriaLaserWeld304LStainlessSteel : public Material
{
public:
  static InputParameters validParams();

  AriaLaserWeld304LStainlessSteel(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Real _c_mu0;
  const Real _c_mu1;
  const Real _c_mu2;
  const Real _c_mu3;
  const Real _Tmax;
  const Real _Tl;
  const Real _T90;
  const Real _beta;
  const Real _c_k0;
  const Real _c_k1;
  const Real _c_cp0;
  const Real _c_cp1;
  const Real _c_rho0;
  const ADVariableValue & _temperature;
  const ADVariableGradient & _grad_temperature;
  ADMaterialProperty<Real> & _mu;
  ADMaterialProperty<Real> & _k;
  ADMaterialProperty<Real> & _cp;
  ADMaterialProperty<Real> & _rho;
  ADMaterialProperty<RealVectorValue> & _grad_k;
};
