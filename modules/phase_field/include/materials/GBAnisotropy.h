/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
