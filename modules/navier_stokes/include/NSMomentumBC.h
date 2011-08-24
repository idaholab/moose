#ifndef NSMOMENTUMBC_H
#define NSMOMENTUMBC_H

#include "NSIntegratedBC.h"
#include "NSViscStressTensorDerivs.h"


// Forward Declarations
class NSMomentumBC;

template<>
InputParameters validParams<NSMomentumBC>();

/**
 * This class corresponds to the "natural" boundary condition
 * for the momentum equations, i.e. what you get if you integrate
 * both the invsicid and viscous flux terms by parts:
 *
 * int_{Gamma} n . (rho*uu + Ip - tau) . v
 *
 * A typical use for this kernel would be a subsonic outflow BC in
 * which one physical value must be specified.  In this case, the
 * residual and Jacobian contrbutions of the n.(rho*u)(u.v) and
 * n.tau.v terms are computed and added to the matrix/rhs.  For the
 * pressure term, the residual contribution due to the specified
 * pressure is computed but there is no corresponding Jacobian entry
 * since the value is given.
 */
class NSMomentumBC : public NSIntegratedBC
{
public:

  NSMomentumBC(const std::string & name, InputParameters parameters);

  virtual ~NSMomentumBC(){}

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

  // An object for computing viscous stress tensor derivatives.
  // Constructed via a reference to ourself so we can access all of our data.
  NSViscStressTensorDerivs<NSMomentumBC> _vst_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSViscStressTensorDerivs;
};

#endif // MOMENTUMBC_H
