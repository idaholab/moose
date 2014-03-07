#include "NSMassSpecifiedNormalFlowBC.h"

template<>
InputParameters validParams<NSMassSpecifiedNormalFlowBC>()
{
  InputParameters params = validParams<NSMassBC>();

  // Required parameters.
  params.addRequiredParam<Real>("rhoun", "The specified value of rho*(u.n) for this boundary");

  return params;
}



NSMassSpecifiedNormalFlowBC::NSMassSpecifiedNormalFlowBC(const std::string & name, InputParameters parameters)
    : NSMassBC(name, parameters),

      // Required parameters
      _rhoun(getParam<Real>("rhoun"))
{
}





Real NSMassSpecifiedNormalFlowBC::computeQpResidual()
{
  return this->qp_residual(_rhoun);
}




Real NSMassSpecifiedNormalFlowBC::computeQpJacobian()
{
  return 0.;
}




Real NSMassSpecifiedNormalFlowBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  return 0.;
}

