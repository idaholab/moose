//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GBAnisotropyBase.h"

// Forward Declarations

/**
 * Function[kappa, gamma, m, L] = parameters (sigma, mob, w_GB, sigma0)
 * Parameter determination method is elaborated in Phys. Rev. B, 78(2), 024113, 2008, by N. Moelans
 * This material uses user-supplied m, kappa and calculates remaining parameters using Eq. (36a) -
 * (36c) from the paper. The interface width is allowed to vary in this material. By maintaining
 * kappa constant and equal for all interfaces, the Allen-Cahn equation is fully variational, as
 * described in Moelans, Acta Mat., 59, 1077-1086, 2011.
 */
class GBWidthAnisotropy : public GBAnisotropyBase
{
public:
  static InputParameters validParams();

  GBWidthAnisotropy(const InputParameters & parameters);

private:
  const Real _mu;
  const Real _kappa;
};
