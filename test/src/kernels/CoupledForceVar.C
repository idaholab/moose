//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledForceVar.h"

registerMooseObject("MooseTestApp", CoupledForceVar);

InputParameters
CoupledForceVar::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<std::map<std::string, Real>>(
      "v_coef", "The coupled variables which provides the force and their coefficients");
  return params;
}

CoupledForceVar::CoupledForceVar(const InputParameters & parameters)
  : Kernel(parameters), _var_coef(getParam<std::map<std::string, Real>>("v_coef"))
{
  for (const auto & [var_name, coef] : _var_coef)
  {
    _vars.push_back(&coupledValueByName(var_name));
    _coefs.push_back(coef);
  }
}

Real
CoupledForceVar::computeQpResidual()
{
  Real v = 0;
  for (const auto i : make_range(_var_coef.size()))
    v += _coefs[i] * (*_vars[i])[_qp];
  return -v * _test[_i][_qp];
}

Real
CoupledForceVar::computeQpJacobian()
{
  return 0;
}

Real
CoupledForceVar::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  mooseError("This is only for testing variable coupling thus full Jacobian evaluation is not "
             "implemented");
  return 0.0;
}
