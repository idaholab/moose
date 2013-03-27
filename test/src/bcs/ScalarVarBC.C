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

#include "ScalarVarBC.h"

template<>
InputParameters validParams<ScalarVarBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("alpha", "The scalar variable coupled in");
  return params;
}

ScalarVarBC::ScalarVarBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _alpha_var(coupledScalar("alpha")),
    _alpha(coupledScalarValue("alpha"))
{
}

Real
ScalarVarBC::computeQpResidual()
{
  return -_alpha[0] * _test[_i][_qp];
}

Real
ScalarVarBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _alpha_var)
    return -_test[_i][_qp];
  else
    return 0.;
}
