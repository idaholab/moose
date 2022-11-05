//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADScalarLMKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

namespace
{
const InputParameters &
setADScalarLMKParam(const InputParameters & params_in)
{
  // Reset the scalar_variable parameter to a relevant name for this physics
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  ret.set<VariableName>("scalar_variable") = {params_in.get<VariableName>("kappa")};
  return ret;
}
}

registerMooseObject("MooseApp", ADScalarLMKernel);

InputParameters
ADScalarLMKernel::validParams()
{
  InputParameters params = ADKernelScalarBase::validParams();
  params.addClassDescription("This class is used to enforce integral of phi = V_0 with a "
                             "Lagrange multiplier approach.");
  params.addRequiredParam<VariableName>("kappa", "Primary coupled scalar variable");
  params.addRequiredParam<PostprocessorName>(
      "pp_name", "Name of the Postprocessor containing the volume of the domain.");
  params.addRequiredParam<Real>(
      "value", "Given (constant) which we want the integral of the solution variable to match.");

  return params;
}

ADScalarLMKernel::ADScalarLMKernel(const InputParameters & parameters)
  : ADKernelScalarBase(setADScalarLMKParam(parameters)),
    _value(getParam<Real>("value")),
    _pp_value(getPostprocessorValue("pp_name"))
{
}

ADScalarLMKernel::~ADScalarLMKernel() {}

ADReal
ADScalarLMKernel::computeQpResidual()
{
  return _kappa[0] * _test[_i][_qp];
}

ADReal
ADScalarLMKernel::computeScalarQpResidual()
{
  return _u[_qp] - _value / _pp_value;
}
