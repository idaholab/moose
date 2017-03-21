/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMassSpecifiedNormalFlowBC.h"

template <>
InputParameters
validParams<NSMassSpecifiedNormalFlowBC>()
{
  InputParameters params = validParams<NSMassBC>();
  params.addClassDescription("This class implements the mass equation boundary term with a "
                             "specified value of rho*(u.n) imposed weakly.");
  params.addRequiredParam<Real>("rhoun", "The specified value of rho*(u.n) for this boundary");
  return params;
}

NSMassSpecifiedNormalFlowBC::NSMassSpecifiedNormalFlowBC(const InputParameters & parameters)
  : NSMassBC(parameters), _rhoun(getParam<Real>("rhoun"))
{
}

Real
NSMassSpecifiedNormalFlowBC::computeQpResidual()
{
  return qpResidualHelper(_rhoun);
}

Real
NSMassSpecifiedNormalFlowBC::computeQpJacobian()
{
  return 0.0;
}

Real
NSMassSpecifiedNormalFlowBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  return 0.0;
}
