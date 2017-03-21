/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMASSBC_H
#define NSMASSBC_H

#include "NSIntegratedBC.h"

// Forward Declarations
class NSMassBC;

template <>
InputParameters validParams<NSMassBC>();

/**
 * This class corresponds to the "natural" boundary condition
 * for the mass equation, i.e. what you get if you integrate
 * the invsicid flux term by parts:
 *
 * int_{Gamma} (rho*u.n) v
 *
 * While this class implements the residual and jacobian values for
 * this term, it does not itself implement any of the computeQp*
 * functions.  For that, use one of the derived classes:
 * 1.) NSMassSpecifiedNormalFlowBC
 * 2.) NSMassUnspecifiedNormalFlowBC
 */
class NSMassBC : public NSIntegratedBC
{
public:
  NSMassBC(const InputParameters & parameters);

protected:
  /**
   * Compute the residual contribution for a given value of
   * rho*(u.n).  This value may come from the current nonlinear
   * solution or be specified, depending on the derived class.
   */
  Real qpResidualHelper(Real rhoun);

  /**
   * Compute the Jacobian contribution due to variable
   * number 'var_number'.  Note: if this is a specified
   * normal flow boundary, the Jacobian will be zero.
   */
  Real qpJacobianHelper(unsigned var_number);
};

#endif // MASSBC_H
