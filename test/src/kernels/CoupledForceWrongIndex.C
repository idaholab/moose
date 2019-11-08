//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledForceWrongIndex.h"

registerMooseObject("MooseTestApp", CoupledForceWrongIndex);

InputParameters
CoupledForceWrongIndex::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addCoupledVar("v", {1, 2}, "The two coupled variables which provide the force");
  params.addParam<bool>("access_value",
                        true,
                        "If true accesses the value and not the var index. Switch to test both "
                        "coupled and coupledValue");
  return params;
}

CoupledForceWrongIndex::CoupledForceWrongIndex(const InputParameters & parameters)
  : Kernel(parameters)
{
  _v_var.resize(coupledComponents("v"));
  _v.resize(coupledComponents("v"));

  // with default values this will error out
  for (unsigned int j = 0; j < 3; ++j)
  {
    if (getParam<bool>("access_value"))
    {
      const VariableValue * temp = &coupledValue("v", j);
      _v[j] = temp;
    }
    else
    {
      unsigned int k = coupled("v", j);
      _v_var[j] = k;
    }
  }
}

Real
CoupledForceWrongIndex::computeQpResidual()
{
  Real s = 0;
  for (unsigned int j = 0; j < _v.size(); ++j)
    s += (*_v[j])[_qp];
  return -s * _test[_i][_qp];
}

Real
CoupledForceWrongIndex::computeQpJacobian()
{
  return 0;
}
