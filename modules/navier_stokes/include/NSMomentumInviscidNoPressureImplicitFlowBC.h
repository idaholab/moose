#ifndef NSMOMENTUMINVISCIDNOPRESSUREIMPLICITFLOWBC_H
#define NSMOMENTUMINVISCIDNOPRESSUREIMPLICITFLOWBC_H

#include "NSMomentumInviscidBC.h"


// Forward Declarations
class NSMomentumInviscidNoPressureImplicitFlowBC;

template<>
InputParameters validParams<NSMomentumInviscidNoPressureImplicitFlowBC>();

/**
 * Momentum equation boundary condition in which pressure is specified (given)
 * and the value of the convective part is allowed to vary (is computed implicitly).
 */
class NSMomentumInviscidNoPressureImplicitFlowBC : public NSMomentumInviscidBC
{
public:
  NSMomentumInviscidNoPressureImplicitFlowBC(const std::string & name, InputParameters parameters);

  virtual ~NSMomentumInviscidNoPressureImplicitFlowBC(){}

protected:
  
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif // NSMOMENTUMINVISCIDNOPRESSUREIMPLICITFLOWBC_H
