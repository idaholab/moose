//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RealVectorFunctionControl.h"
#include "Function.h"

registerMooseObject("MooseTestApp", RealVectorFunctionControl);

InputParameters
RealVectorFunctionControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription("Sets all component a 'RealEigenVector' input parameters to the value "
                             "of a provided function.");
  params.addRequiredParam<FunctionName>(
      "function", "The function to use for controlling the specified parameter.");
  params.addRequiredParam<std::string>(
      "parameter",
      "The input parameter(s) to control. Specify a single parameter name and all "
      "parameters in all objects matching the name will be updated");
  return params;
}

RealVectorFunctionControl::RealVectorFunctionControl(const InputParameters & parameters)
  : Control(parameters), _function(getFunction("function"))
{
}

void
RealVectorFunctionControl::execute()
{
  Real value = _function.value(_t);
  auto cname = getParam<std::string>("parameter");
  unsigned int ncomp = getControllableValueByName<RealEigenVector>(cname).size();
  setControllableValueByName<RealEigenVector>(cname, RealEigenVector::Constant(ncomp, value));
}
