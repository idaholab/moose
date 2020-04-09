//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSMassSpecifiedNormalFlowBC.h"

registerMooseObject("NavierStokesApp", NSMassSpecifiedNormalFlowBC);

InputParameters
NSMassSpecifiedNormalFlowBC::validParams()
{
  InputParameters params = NSMassBC::validParams();
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
