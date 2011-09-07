#ifndef NSENERGYINVISCIDBC_H
#define NSENERGYINVISCIDBC_H

#include "NSIntegratedBC.h"
#include "NSPressureDerivs.h"

// Forward Declarations
class NSEnergyInviscidBC;

template<>
InputParameters validParams<NSEnergyInviscidBC>();


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
 */
class NSEnergyInviscidBC : public NSIntegratedBC
{

public:
  NSEnergyInviscidBC(const std::string & name, InputParameters parameters);

  virtual ~NSEnergyInviscidBC(){}

protected:
  
  /**
   * Must be implemented in derived classes.
   */
//  virtual Real computeQpResidual();
//  virtual Real computeQpJacobian();
//  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // The specified value of the pressure.  This is used in
  // the subsonic outflow boundary condition.
  // Real _specified_pressure;

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
  Real qp_residual(Real pressure, Real un);

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
  Real qp_jacobian_termA(unsigned var_number, Real pressure);

  // d(U4)/dX * (u.n)
  Real qp_jacobian_termB(unsigned var_number, Real un);

  // d(p)/dX * (u.n)
  Real qp_jacobian_termC(unsigned var_number, Real un);
};


#endif // NSENERGYINVISCIDBC_H
