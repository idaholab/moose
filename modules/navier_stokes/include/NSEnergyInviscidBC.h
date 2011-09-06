#ifndef NSENERGYINVISCIDBC_H
#define NSENERGYINVISCIDBC_H

#include "NSIntegratedBC.h"

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
 * A typical use for this kernel would be a subsonic outflow BC in
 * which one physical value (pressure) is specified.  For the
 * enthalpy term, rho*H = rho*E + p, and the residual and Jacobian 
 * contributions are computed as normal except p is treated as given.
 */
class NSEnergyInviscidBC : public NSIntegratedBC
{

public:
  NSEnergyInviscidBC(const std::string & name, InputParameters parameters);

  virtual ~NSEnergyInviscidBC(){}

protected:
  
  /**
   * Just like other kernels, we must overload the Residual and Jacobian contributions...
   */
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // The specified value of the pressure.  This is used in
  // the subsonic outflow boundary condition.
  Real _specified_pressure;
};


#endif // NSENERGYINVISCIDBC_H
