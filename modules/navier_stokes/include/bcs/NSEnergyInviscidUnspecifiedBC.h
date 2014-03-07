#ifndef NSENERGYINVISCIDUNSPECIFIEDBC_H
#define NSENERGYINVISCIDUNSPECIFIEDBC_H

#include "NSEnergyInviscidBC.h"

// Forward Declarations
class NSEnergyInviscidUnspecifiedBC;

template<>
InputParameters validParams<NSEnergyInviscidUnspecifiedBC>();


/**
 * The inviscid energy BC term with specified pressure.
 */
class NSEnergyInviscidUnspecifiedBC : public NSEnergyInviscidBC
{

public:
  NSEnergyInviscidUnspecifiedBC(const std::string & name, InputParameters parameters);

  virtual ~NSEnergyInviscidUnspecifiedBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Aux Variables
  VariableValue& _pressure;

private:
  // Helper Jacobian function
  Real compute_jacobian(unsigned var_number);
};

#endif // NSENERGYINVISCIDUNSPECIFIEDBC_H
