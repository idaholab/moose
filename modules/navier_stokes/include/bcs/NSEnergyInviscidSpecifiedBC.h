#ifndef NSENERGYINVISCIDSPECIFIEDBC_H
#define NSENERGYINVISCIDSPECIFIEDBC_H

#include "NSEnergyInviscidBC.h"

// Forward Declarations
class NSEnergyInviscidSpecifiedBC;

template<>
InputParameters validParams<NSEnergyInviscidSpecifiedBC>();


/**
 * The inviscid energy BC term with specified pressure.
 */
class NSEnergyInviscidSpecifiedBC : public NSEnergyInviscidBC
{

public:
  NSEnergyInviscidSpecifiedBC(const std::string & name, InputParameters parameters);

  virtual ~NSEnergyInviscidSpecifiedBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  Real _specified_pressure;
  Real _un;

private:
  // Helper Jacobian function
  Real compute_jacobian(unsigned var_number);
};

#endif // NSENERGYINVISCIDSPECIFIEDPRESSUREBC_H
