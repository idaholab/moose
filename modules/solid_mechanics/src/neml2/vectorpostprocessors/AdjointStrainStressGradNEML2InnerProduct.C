//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdjointStrainStressGradNEML2InnerProduct.h"

#include "NEML2Utils.h"

registerMooseObject("SolidMechanicsApp", AdjointStrainStressGradNEML2InnerProduct);

InputParameters
AdjointStrainStressGradNEML2InnerProduct::validParams()
{
  InputParameters params = ElementOptimizationFunctionInnerProduct::validParams();
  params.addClassDescription(
      "This component is designed to compute the gradient of the objective function concerning "
      "specific properties. It achieves this by computing the inner product of the property "
      "derivative obtained from a NEML2 material model and the strain resulting from the forward "
      "simulation");
  params.addRequiredParam<UserObjectName>("stress_derivative",
                                          "The user object that stores the stress derivative");
  params.addRequiredParam<MaterialPropertyName>(
      "adjoint_strain_name", "Name of the strain property in the adjoint problem");
  return params;
}

AdjointStrainStressGradNEML2InnerProduct::AdjointStrainStressGradNEML2InnerProduct(
    const InputParameters & parameters)
  : ElementOptimizationFunctionInnerProduct(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _adjoint_strain(getMaterialPropertyByName<RankTwoTensor>(
        getParam<MaterialPropertyName>("adjoint_strain_name"))),
    _derivative_uo(getUserObject<BatchPropertyDerivativeRankTwoTensorReal>("stress_derivative")),
    _derivative(_derivative_uo.getOutputData())
{
  NEML2Utils::libraryNotEnabledError(parameters);
}

Real
AdjointStrainStressGradNEML2InnerProduct::computeQpInnerProduct()
{
  if (!_derivative_uo.outputReady())
    mooseError("The NEML2 material update has not been performed yet");

  const auto index = _derivative_uo.getIndex(_current_elem->id());
  Real ans = -_adjoint_strain[_qp].doubleContraction(_derivative[index + _qp]);

  return ans;
}
