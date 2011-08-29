#ifndef NSMASSBC_H
#define NSMASSBC_H

#include "NSIntegratedBC.h"


// Forward Declarations
class NSMassBC;

template<>
InputParameters validParams<NSMassBC>();

/**
 * This class corresponds to the "natural" boundary condition
 * for the mass equation, i.e. what you get if you integrate
 * the invsicid flux term by parts:
 *
 * int_{Gamma} (rho*u.n) v
 *
 * A typical use for this kernel would be a subsonic outflow BC in
 * which one physical value must be specified.  In this case, the
 * residual and Jacobian contributions from the term above are computed
 * and added to the matrix and residual vectors.
 */
class NSMassBC : public NSIntegratedBC
{
public:

  NSMassBC(const std::string & name, InputParameters parameters);

  virtual ~NSMassBC(){}

protected:
  
  /**
   * Just like other kernels, we must overload the Residual and Jacobian contributions...
   */
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

};

#endif // MASSBC_H
