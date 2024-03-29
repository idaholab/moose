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
#include "FEProblemBase.h"
#include "SubProblem.h"

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

/**
 * Finds the non-dimensional wall distance normalized with the friction velocity
 * Implements a fixed-point iteration in the wall function to get this velocity
 * @param mu the dynamic viscosity
 * @param rho the density
 * @param u the centroid velocity
 * @param dist the element centroid distance to the wall
 * @return the non-dimensional wall distance
 */
ADReal findyPlus(const ADReal & mu, const ADReal & rho, const ADReal & u, Real dist);

using MooseUtils::isZero;

/**
 * Compute the speed (velocity norm) given the supplied velocity
 */
ADReal computeSpeed(const ADRealVectorValue & velocity);

/**
 * Map marking wall bounded elements
 * The map passed in \p wall_bounded_map gets cleared and re-populated
 */
void getWallBoundedElements(const std::vector<BoundaryName> & wall_boundary_name,
                            const FEProblemBase & fe_problem,
                            const SubProblem & subproblem,
                            const std::set<SubdomainID> & block_ids,
                            std::map<const Elem *, bool> & wall_bounded_map);

/**
 * Map storing wall ditance for near-wall marked elements
 * The map passed in \p dist_map gets cleared and re-populated
 */
void getWallDistance(const std::vector<BoundaryName> & wall_boundary_name,
                     const FEProblemBase & fe_problem,
                     const SubProblem & subproblem,
                     const std::set<SubdomainID> & block_ids,
                     std::map<const Elem *, std::vector<Real>> & dist_map);

/**
 * Map storing face arguments to wall bounded faces
 * The map passed in \p face_info_map gets cleared and re-populated
 */
void getElementFaceArgs(const std::vector<BoundaryName> & wall_boundary_name,
                        const FEProblemBase & fe_problem,
                        const SubProblem & subproblem,
                        const std::set<SubdomainID> & block_ids,
                        std::map<const Elem *, std::vector<const FaceInfo *>> & face_info_map);

/**
 * Compute the divergence of a vector given its matrix of derivatives
 */
template <typename T, typename VectorType, typename PointType>
T
divergence(const TensorValue<T> & gradient,
           const VectorType & value,
           const PointType & point,
           const Moose::CoordinateSystemType & coord_sys,
           const unsigned int rz_radial_coord)
{
  mooseAssert((coord_sys == Moose::COORD_XYZ) || (coord_sys == Moose::COORD_RZ),
              "This function only supports calculations of divergence in Cartesian and "
              "axisymmetric coordinate systems");
  auto div = gradient.tr();
  if (coord_sys == Moose::COORD_RZ)
    // u_r / r
    div += value(rz_radial_coord) / point(rz_radial_coord);
  return div;
}
}
