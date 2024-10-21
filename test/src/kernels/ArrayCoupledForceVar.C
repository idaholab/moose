//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayCoupledForceVar.h"

registerMooseObject("MooseTestApp", ArrayCoupledForceVar);

InputParameters
ArrayCoupledForceVar::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addRequiredParam<std::map<std::string, std::string>>(
      "v_coef", "The coupled variables which provides the force and their coefficients");
  return params;
}

ArrayCoupledForceVar::ArrayCoupledForceVar(const InputParameters & parameters)
  : ArrayKernel(parameters), _var_coef(getParam<std::map<std::string, std::string>>("v_coef"))
{
  for (const auto & [var_name, coef_str] : _var_coef)
  {
    _vars.push_back(&coupledArrayValueByName(var_name));

    std::vector<Real> coefs;
    MooseUtils::tokenizeAndConvert(coef_str, coefs, ",");

    ArrayMooseVariable * var = &_fe_problem.getArrayVariable(_tid, var_name);
    unsigned int n = coefs.size();
    if (var->count() != n)
      paramError("v_coef",
                 "Number of coefficients for '",
                 var_name,
                 "' does not match the number of components");
    RealEigenVector c(n);
    for (unsigned int i = 0; i < n; ++i)
      c(i) = coefs[i];
    _coefs.push_back(c);
  }
}

void
ArrayCoupledForceVar::computeQpResidual(RealEigenVector & residual)
{
  Real v = 0;
  for (const auto i : make_range(_var_coef.size()))
    v -= _coefs[i].dot((*_vars[i])[_qp]) * _test[_i][_qp];
  for (unsigned int i = 0; i < _count; ++i)
    residual(i) = v;
}
