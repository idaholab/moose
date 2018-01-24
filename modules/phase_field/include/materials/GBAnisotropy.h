//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GBANISOTROPY_H
#define GBANISOTROPY_H

#include "GBAnisotropyBase.h"

// Forward Declarations
class GBAnisotropy;

template <>
InputParameters validParams<GBAnisotropy>();

/**
 * Function[kappa, gamma, m, L] = parameters (sigma, mob, w_GB, sigma0)
 * Parameter determination method is elaborated in Phys. Rev. B, 78(2), 024113, 2008, by N. Moelans
 * This material uses Algorithm 1 from the paper to determine parameters for constant GB width
 */
class GBAnisotropy : public GBAnisotropyBase
{
public:
  GBAnisotropy(const InputParameters & parameters);

private:
  const Real _wGB;
};

#endif // GBANISOTROPY_H
