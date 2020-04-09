//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSMassUnspecifiedNormalFlowBC.h"

registerMooseObject("NavierStokesApp", NSMassUnspecifiedNormalFlowBC);

InputParameters
NSMassUnspecifiedNormalFlowBC::validParams()
{
  InputParameters params = NSMassBC::validParams();
  params.addClassDescription("This class implements the mass equation boundary term with the "
                             "rho*(u.n) boundary integral computed implicitly.");
  return params;
}

NSMassUnspecifiedNormalFlowBC::NSMassUnspecifiedNormalFlowBC(const InputParameters & parameters)
  : NSMassBC(parameters)
{
}

Real
NSMassUnspecifiedNormalFlowBC::computeQpResidual()
{
  const RealVectorValue mom(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
  return qpResidualHelper(mom * _normals[_qp]);
}

Real
NSMassUnspecifiedNormalFlowBC::computeQpJacobian()
{
  return qpJacobianHelper(/*on diagonal=*/0);
}

Real
NSMassUnspecifiedNormalFlowBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return qpJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}
