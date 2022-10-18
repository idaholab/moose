//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include "Moose.h"
#include "MooseUtils.h"
#include "ADReal.h"
#include "metaphysicl/raw_type.h"

namespace NS
{
/**
 * Delta function, which returns zero if $i\ne j$ and unity if $i=j$
 * @param[in] i   integer number
 * @param[in] j   integer number
 * @return delta function
 */
int delta(unsigned int i, unsigned int j);

/**
 * Sign function, returns $+1$ if $a$ is positive and $-1$ if $a$ is negative
 * @param[in] a   number
 * @return the sign of the input
 */
int computeSign(const Real & a);

/**
 * Determines the index $i$ in a sorted array such that the input point is within
 * the $i$-th and $i+1$-th entries in the array.
 * @param[in] p      input point
 * @param[in] bounds sorted array
 * @return index of point
 */
unsigned int getIndex(const Real & p, const std::vector<Real> & bounds);

/**
 * Computes the derivative of the Reynolds number, $Re\equiv \frac{\rho Vd}{\mu}$,
 * with respect to an arbitrary variable $\zeta$, where it is assumed that only the
 * material properties of density $\rho$ and dynamic viscosity $\mu$ have nonzero
 * derivatives with respect to $\zeta$. To eliminate the need to pass in the velocity $V$ and
 * characteristic length $d$, the derivative is rewritten in terms of the Reynolds
 * number such that the partial derivative of $Re$ with respect to an aritrary
 * parameter $\zeta$ is
 *
 * $\frac{\partial Re}{\partial\zeta}=Re\left(\frac{1}{\rho}\frac{\partial\rho}{\partial
 * x}-\frac{1}{\mu}\frac{\partial\mu}{\partial x}$
 *
 * @param[in] Re   Reynolds number
 * @param[in] rho  density
 * @param[in] mu   dynamic viscosity
 * @param[in] drho partial derivative of density with respect to arbitrary variable $\zeta$
 * @param[in] dmu  partial derivative of dynamic viscosity with respect to arbitrary variable
 * $\zeta$
 * @return derivative of Reynolds number with respect to $\zeta$
 */
Real reynoldsPropertyDerivative(
    const Real & Re, const Real & rho, const Real & mu, const Real & drho, const Real & dmu);

/**
 * Computes the derivative of the Prandtl number, $Pr\equiv\frac{\mu C_p}{k}$, with respect
 * to an arbitrary variale $\zeta$. This derivative is
 *
 * $\frac{\partial Pr}{\partial \zeta}=\frac{k\left(\mu\frac{\partial
 * C_p}{\partial\zeta}+C_p\frac{\partial mu}{\partial\zeta}\right)-\mu C_p\frac{\partial
 * k}{\partial\zeta}}{k^2}$
 *
 * @param[in] mu  dynamic viscosity
 * @param[in] cp  isobaric specific heat
 * @param[in] k   thermal conductivity
 * @param[in] dmu derivative of dynamic viscosity with respect to $\zeta$
 * @param[in] dcp derivative of isobaric specific heat with respect to $\zeta$
 * @param[in] dk  derivative of thermal conductivity with respect to $\zeta$
 * @return derivative of Prandtl number with respect to $\zeta$
 */
Real prandtlPropertyDerivative(const Real & mu,
                               const Real & cp,
                               const Real & k,
                               const Real & dmu,
                               const Real & dcp,
                               const Real & dk);

/**
 * Finds the friction velocity using standard velocity wall functions formulation.
 * It is used in WallFunctionWallShearStressAux, WallFunctionYPlusAux and
 * INSFVWallFunctionBC.
 * @param mu the dynamic viscosity
 * @param rho the density
 * @param u the centroid velocity
 * @param dist the element centroid distance to the wall
 * @return the velocity at the wall
 */
ADReal findUStar(const ADReal & mu, const ADReal & rho, const ADReal & u, Real dist);

using MooseUtils::isZero;

/**
 * Compute the speed (velocity norm) given the supplied velocity
 */
ADReal computeSpeed(const ADRealVectorValue & velocity);
}
