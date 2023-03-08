//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultipleUpdateAux.h"

registerMooseObject("MooseTestApp", MultipleUpdateAux);

InputParameters
MultipleUpdateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredCoupledVar("u", "unknown (nl-variable)");
  params.addRequiredCoupledVar("var1", "an aux variable to update");
  params.addRequiredCoupledVar("var2", "another aux variable to update");
  params.addParam<bool>("use_deprecated_api", false, "Test the deprecated API");
  return params;
}

MultipleUpdateAux::MultipleUpdateAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _nl_u(coupledValue("u")),
    _deprecated(getParam<bool>("use_deprecated_api"))
{
  if (_deprecated)
  {
    _dvar1 = &writableCoupledValue("var1");
    _dvar2 = &writableCoupledValue("var2");
  }
  else
  {
    _var1 = &writableVariable("var1");
    _var2 = &writableVariable("var2");
  }
}

MultipleUpdateAux::~MultipleUpdateAux() {}

Real
MultipleUpdateAux::computeValue()
{
  if (_deprecated)
  {
    (*_dvar1)[_qp] = _nl_u[_qp] + 10.0;
    (*_dvar2)[_qp] = _nl_u[_qp] + 200.0;
  }
  else
  {
    _var1->setNodalValue(_nl_u[_qp] + 10.0, _qp);
    _var2->setNodalValue(_nl_u[_qp] + 200.0, _qp);
  }
  return -3.33;
}
