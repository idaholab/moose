/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GBWIDTHANISOTROPY_H
#define GBWIDTHANISOTROPY_H

#include "GBAnisotropyBase.h"

// Forward Declarations
class GBWidthAnisotropy;

template <>
InputParameters validParams<GBWidthAnisotropy>();

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
  GBWidthAnisotropy(const InputParameters & parameters);

private:
  const Real _mu;
  const Real _kappa;
};

#endif // GBWIDTHANISOTROPY_H
