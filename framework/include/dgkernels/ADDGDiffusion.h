//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

/**
 * DG kernel for diffusion
 *
 * General DG kernel that this class can handle is:
 * \f$ { \nabla u * n_e} [v] + epsilon { \nabla v * n_e } [u] + (sigma / |e| * [u][v]) \f$
 *
 * \f$ [a] = [ a_1 - a_2 ] \f$
 * \f$ {a} = 0.5 * (a_1 + a_2) \f$
 *
 */
class ADDGDiffusion : public ADDGKernel
{
public:
  static InputParameters validParams();

  ADDGDiffusion(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  Real _epsilon;
  Real _sigma;
  const ADMaterialProperty<Real> & _diff;
  const ADMaterialProperty<Real> & _diff_neighbor;
};
