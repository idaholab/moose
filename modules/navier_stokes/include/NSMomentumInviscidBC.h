#ifndef NSMOMENTUMINVISCIDBC_H
#define NSMOMENTUMINVISCIDBC_H

#include "NSIntegratedBC.h"
#include "NSPressureDerivs.h"


// Forward Declarations
class NSMomentumInviscidBC;

template<>
InputParameters validParams<NSMomentumInviscidBC>();

/**
 * This class corresponds to the inviscid part of the "natural"
 * boundary condition for the momentum equations, i.e. 
 *
 * int_{Gamma} n . (rho*uu + Ip) . v
 *
 * A typical use for this kernel would be a subsonic outflow BC in the
 * Euler or Navier-Stokes equations in which one physical value (the
 * pressure) is specified.  In this case, the residual and Jacobian
 * contrbutions of the n.(rho*u)(u.v) term is computed and added to
 * the matrix/rhs.  For the pressure term, the residual contribution
 * due to the specified pressure is computed but there is no
 * corresponding Jacobian entry since the value is given.
 */
class NSMomentumInviscidBC : public NSIntegratedBC
{
public:

  NSMomentumInviscidBC(const std::string & name, InputParameters parameters);

  virtual ~NSMomentumInviscidBC(){}

protected:
  
  /**
   * Must be implemented in derived classes.
   */
//  virtual Real computeQpResidual();
//  virtual Real computeQpJacobian();
//  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Aux Var - TODO: We may want this if the pressure is not specified... not sure what 
  // type of boundary condition that would be (specified outflow would use a Dirichlet
  // BC...)
  // VariableValue & _pressure; 
  
  // Which spatial component of the momentum equations (0,1, or 2) is this
  // kernel applied in?
  unsigned _component;

  // Ratio of specific heats
  Real _gamma;

  // The specified value of the pressure.  This is used in
  // the subsonic outflow boundary condition.
  // FIXME: This should move down into classes where it is required.
  // Real _specified_pressure;

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
  // .) Fully specified (both pressure and (rho*u)(u.n) given, this may not be physically meaningful?)

  // Depending on the passed-in value, will compute the residual for either a specified
  // pressure value or the residual at the current value of the pressure.
  Real pressure_qp_residual(Real pressure);

  // If the pressure is fixed, the Jacobian of the pressure term is zero, otherwise
  // we return the Jacobian value for the passed-in variable number.
  Real pressure_qp_jacobian(unsigned var_number);

  // Depending on the passed-in vector, will compute the residual for either a specified
  // value of (rho*u)(u.n) or the residual at the current value of (rho*u)(u.n).
  // The passed-in value is the _component'th entry of the (rho*u)(u.n) vector.
  Real convective_qp_residual(Real rhou_udotn);

  // If the value of (rho*u)(u.n) is fixed, the Jacobian of the
  // convective term is zero, otherwise we return the correct value
  // based on the passed-in variable number.
  Real convective_qp_jacobian(unsigned var_number);
};

#endif // NSMOMENTUMINVISCIDBC_H
