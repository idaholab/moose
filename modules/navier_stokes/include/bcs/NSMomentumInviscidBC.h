//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSIntegratedBC.h"
#include "NSPressureDerivs.h"

// Forward Declarations

/**
 * This class corresponds to the inviscid part of the "natural"
 * boundary condition for the momentum equations, i.e.
 *
 * int_{Gamma} n . (rho*uu + Ip) . v
 *
 * While this kernel implements the convective and pressure term
 * residuals and jacobians, it does not itself implement any of
 * the computeQp* functions.  For that, use one of the derived
 * classes:
 * 1.) NSMomentumInviscidSpecifiedPressureBC
 * 2.) NSMomentumInviscidSpecifiedNormalFlowBC
 * 3.) NSMomentumInviscidNoPressureImplicitFlowBC
 *
 * The first kernel above would be used for a subsonic outflow BC in
 * the Euler or Navier-Stokes equations in which one physical value
 * (the pressure) is specified.  In this case, the residual and
 * Jacobian contrbutions of the n.(rho*u)(u.v) term are computed and
 * added to the matrix/rhs.  For the pressure term, the residual
 * contribution due to the specified pressure is computed but there is
 * no corresponding Jacobian entry since the value is given.
 *
 * The second kernel above would be used if, instead of the pressure,
 * the value of the vector (rho*u)(u.n) is given.  This situation is
 * not common, but a special case of it, u.n=0, is very common for
 * implementing a free-slip boundary: in that situation the pressure
 * is an unknown and u.n = 0 would be imposed by the kernel (though in
 * this trivial case you could omit the term completely).
 *
 * The third kernel above would be used when the pressure term
 * has *not* been integrated by parts in the momentum equations, and
 * therefore there is no pressure in the boundary term at all.
 *
 * We note that other combinations are also theoretically possible
 * (e.g. unspecified pressure and normal flow, fully-specified
 * pressure and normal flow) however they have not yet been implemented
 * since I'm not sure if they are physically-relevant.
 */
class NSMomentumInviscidBC : public NSIntegratedBC
{
public:
  static InputParameters validParams();

  NSMomentumInviscidBC(const InputParameters & parameters);

protected:
  // Which spatial component of the momentum equations (0,1, or 2) is this
  // kernel applied in?
  const unsigned _component;

  // An object for computing pressure derivatives.
  // Constructed via a reference to ourself
  NSPressureDerivs<NSMomentumInviscidBC> _pressure_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSPressureDerivs;

  // These functions can be mix-n-matched by derived classes to implement
  // any of the following boundary conditions:
  // .) Fully unspecified (both (rho*u)(u.n) and p computed implicitly, is this valid?)
  // .) Specified pressure/unspecified (rho*u)(u.n)
  // .) Unspecified pressure/specified (rho*u)(u.n)
  // .) Fully specified (both pressure and (rho*u)(u.n) given, this may not be physically
  // meaningful?)

  // Depending on the passed-in value, will compute the residual for either a specified
  // pressure value or the residual at the current value of the pressure.
  Real pressureQpResidualHelper(Real pressure);

  // If the pressure is fixed, the Jacobian of the pressure term is zero, otherwise
  // we return the Jacobian value for the passed-in variable number.
  Real pressureQpJacobianHelper(unsigned var_number);

  // Depending on the passed-in vector, will compute the residual for either a specified
  // value of (rho*u)(u.n) or the residual at the current value of (rho*u)(u.n).
  // The passed-in value is the _component'th entry of the (rho*u)(u.n) vector.
  Real convectiveQpResidualHelper(Real rhou_udotn);

  // If the value of (rho*u)(u.n) is fixed, the Jacobian of the
  // convective term is zero, otherwise we return the correct value
  // based on the passed-in variable number.
  Real convectiveQpJacobianHelper(unsigned var_number);
};
