//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

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
class AriaLaserWeld304LStainlessSteelFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  AriaLaserWeld304LStainlessSteelFunctorMaterial(const InputParameters & parameters);

protected:
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
  const Real _ap0;
  const Real _ap1;
  const Real _ap2;
  const Real _ap3;
  const Real _bp0;
  const Real _bp1;
  const Real _bp2;
  const Real _bp3;
  const Real _Tb;
  const Real _Tbound1;
  const Real _Tbound2;
  const Moose::Functor<ADReal> & _temperature;
};
