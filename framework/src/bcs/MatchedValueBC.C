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

#include "MatchedValueBC.h"

template <>
InputParameters
validParams<MatchedValueBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredCoupledVar("v", "The variable whose value we are to match.");
  return params;
}

MatchedValueBC::MatchedValueBC(const InputParameters & parameters)
  : NodalBC(parameters), _v(coupledValue("v")), _v_num(coupled("v"))
{
}

Real
MatchedValueBC::computeQpResidual()
{
  return _u[_qp] - _v[_qp];
}

Real
MatchedValueBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_num)
    return -1.0;
  else
    return 0.;
}
