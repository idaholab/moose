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
 * boundary condition for the energy equation, i.e.
 *
 * int_{Gamma} n . (rho*Hu) v
 *
 * While this class implements the residual and jacobian values for
 * this term, it does not itself implement any of the computeQp*
 * functions.  For that, use one of the derived classes:
 * 1.) NSEnergyInviscidSpecifiedPressureBC
 * 2.) NSEnergyInviscidSpecifiedNormalFlowBC
 * 3.) NSEnergyInviscidUnspecifiedBC
 * 4.) NSEnergyInviscidSpecifiedBC
 * 5.) NSEnergyInviscidSpecifiedDensityAndVelocityBC
 */
class NSEnergyInviscidBC : public NSIntegratedBC
{
public:
  static InputParameters validParams();

  NSEnergyInviscidBC(const InputParameters & parameters);

protected:
  // Aux vars
  const VariableValue & _temperature;

  // An object for computing pressure derivatives.
  // Constructed via a reference to ourself
  NSPressureDerivs<NSEnergyInviscidBC> _pressure_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSPressureDerivs;

  // Given the two inputs: pressure and u.n, compute the residual
  // at this quadrature point.  Note that the derived classes are
  // responsible for determining whether the inputs are specified
  // values or come from the current solution.
  Real qpResidualHelper(Real pressure, Real un);

  // This was experimental code and did not really work out, do not use!
  // New version, allows input of three variables to provide both:
  // .) specified (rho, u) boundary residuals
  // .) specified pressure boundary residuals
  //
  // The actual term implemented here is:
  // rho*H*(u.n) = (rho*E + p)(u.n) = (rho*(cv*T + 0.5*|u|^2) + p)(u.n)
  Real qpResidualHelper(Real rho, RealVectorValue u, Real pressure);

  // The Jacobian of this term is given by the product rule, i.e.
  //
  // d/dX (U4 + p)(u.n) = (U4+p) * d(u.n)/dX + d(U4+p)/dX * (u.n)
  //                    = (U4+p) * d(u.n)/dX (A)
  //                    + d(U4)/dX * (u.n)   (B)
  //                    + d(p)/dX * (u.n)    (C)
  // For some arbitrary variable X.  We consider 2 cases:
  // 1.) Specified pressure boundary: Term C is zero.
  // 2.) Specified u.n boundary: Term A is zero.
  //
  // This class implements three jacobian functions corresponding to
  // the terms above.  It is up to the derived classes to determine
  // which to call, and which values (specified or variable) must be
  // passed in.

  // (U4+p) * d(u.n)/dX
  Real qpJacobianTermA(unsigned var_number, Real pressure);

  // d(U4)/dX * (u.n)
  Real qpJacobianTermB(unsigned var_number, Real un);

  // d(p)/dX * (u.n)
  Real qpJacobianTermC(unsigned var_number, Real un);

  // The residual term with rho*E expanded has 3 parts:
  // rho*cv*T*(u.n) + rho*0.5*|u|^2*(u.n) + p*(u.n)
  // Each of these terms, when differentiated, leads to
  // multiple terms due to the product rule:
  // (1) d/dX (rho*cv*T*(u.n))      = cv * (d(rho)/dX*T*(u.n) + rho*d(T)/dX*(u.n) + rho*T*d(u.n)/dX)
  // (2) d/dX (rho*0.5*|u|^2*(u.n)) = 0.5 * (d(rho)/dX*|u|^2*(u.n) + rho*d(|u|^2)/dX*(u.n) +
  // rho*|u|^2*d(u.n)/dX)
  // (3) d/dX (p*(u.n)) = d(p)/dx*(u.n) + p*d(u.n)/dX
};
