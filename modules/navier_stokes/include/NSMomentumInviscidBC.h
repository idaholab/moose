#ifndef NSMOMENTUMINVISCIDBC_H
#define NSMOMENTUMINVISCIDBC_H

#include "NSIntegratedBC.h"


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
   * Just like other kernels, we must overload the Residual and Jacobian contributions...
   */
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Aux Var - TODO: We may want this if the pressure is not specified... not sure what 
  // type of boundary condition that would be (specified outflow would use a Dirichlet
  // BC...)
  // VariableValue & _pressure; 
  
  // Which spatial component of the momentum equations (0,1, or 2) is this
  // kernel applied in?
  unsigned _component;
};

#endif // MOMENTUMINVISCIDBC
