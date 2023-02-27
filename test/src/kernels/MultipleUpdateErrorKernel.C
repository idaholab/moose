//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultipleUpdateErrorKernel.h"

registerMooseObject("MooseTestApp", MultipleUpdateErrorKernel);

InputParameters
MultipleUpdateErrorKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("var1", "an aux variable to update");
  params.addRequiredCoupledVar("var2", "another aux variable to update");
  params.addParam<bool>("use_deprecated_api", false, "Test the deprecated API");
  return params;
}

MultipleUpdateErrorKernel::MultipleUpdateErrorKernel(const InputParameters & parameters)
  : Kernel(parameters), _deprecated(getParam<bool>("use_deprecated_api"))
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

Real
MultipleUpdateErrorKernel::computeQpResidual()
{
  mooseError("This should never be reached");
}
