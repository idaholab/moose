//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVGradientAux.h"
#include "LinearFVGradientReader.h"
#include "MooseLinearVariableFV.h"

registerMooseObject("MooseApp", LinearFVGradientAux);

namespace
{
/// Register and return the gradient reader requested by the parameters of a LinearFVGradientAux.
const LinearFVGradientReader &
requestGradient(LinearFVGradientAux & aux, MooseVariableFieldBase * const variable)
{
  auto * const linear_variable = dynamic_cast<MooseLinearVariableFVReal *>(variable);
  if (!linear_variable)
    aux.paramError("linear_fv_variable",
                   "The variable must be a scalar linear finite-volume variable.");

  return aux.isParamValid("gradient_method")
             ? linear_variable->requestCellGradients(
                   aux.getParam<GradientMethodName>("gradient_method"))
             : linear_variable->requestCellGradients();
}
}

InputParameters
LinearFVGradientAux::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription("Copies the cell-centered gradient of a linear finite-volume "
                             "variable, computed with a selected gradient method, into a vector "
                             "auxiliary variable.");
  params.addRequiredCoupledVar("linear_fv_variable",
                               "Linear finite-volume variable whose gradient is copied.");
  params.addParam<GradientMethodName>(
      "gradient_method",
      "Gradient method whose result is copied. Defaults to the gradient method of the variable.");
  return params;
}

LinearFVGradientAux::LinearFVGradientAux(const InputParameters & parameters)
  : VectorAuxKernel(parameters),
    _gradient(requestGradient(*this, getFieldVar("linear_fv_variable", 0)))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports elemental fields.");
}

RealVectorValue
LinearFVGradientAux::computeValue()
{
  mooseAssert(_qp == 0, "Only one quadrature point per element is expected.");
  return _gradient.gradient(_mesh.elemInfo(_current_elem->id()));
}
