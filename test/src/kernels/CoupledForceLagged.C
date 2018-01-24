/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CoupledForceLagged.h"

template <>
InputParameters
validParams<CoupledForceLagged>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("v", "The coupled variable which provides the force");
  return params;
}

CoupledForceLagged::CoupledForceLagged(const InputParameters & parameters)
  : Kernel(parameters), _v_var(coupled("v")), _v(coupledValuePreviousNL("v"))
{
}

Real
CoupledForceLagged::computeQpResidual()
{
  return -_v[_qp] * _test[_i][_qp];
}

Real
CoupledForceLagged::computeQpJacobian()
{
  return 0;
}

Real
CoupledForceLagged::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  // No off-diagonal contribution, becuase v is lagged in newton iterate
  return 0.0;
}
