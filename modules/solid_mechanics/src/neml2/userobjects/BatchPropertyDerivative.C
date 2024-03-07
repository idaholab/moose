//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BatchPropertyDerivative.h"

registerMooseObject("SolidMechanicsApp", BatchPropertyDerivativeRankTwoTensorReal);

template <typename OutputType, typename InputType>
InputParameters
BatchPropertyDerivative<OutputType, InputType>::validParams()
{
  auto params = BatchPropertyDerivativeBase<OutputType, InputType>::validParams();
  params.addClassDescription("Gather material property for NEML2 material model, and receive the "
                             "property derivatives from the NEML2 material model.");
  params.template addRequiredParam<MaterialPropertyName>("material_property",
                                                         "Name of the scalar material property.");
  params.setDocString("execution_order_group",
                      "BatchPropertyDerivative UserObject needs to be completely executed before "
                      "vectorPostprocessors.");

  params.template set<int>("execution_order_group") = -1;

  // keep consistent with CauchyStressFromNEML2UO
  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR};
  params.template set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

template <typename OutputType, typename InputType>
BatchPropertyDerivative<OutputType, InputType>::BatchPropertyDerivative(
    const InputParameters & params)
  : BatchPropertyDerivativeBase<OutputType, InputType>(
        params,
        // here we pass the material property that we are trying to convert to BatchMaterial
        "material_property")
{
}

template class BatchPropertyDerivative<RankTwoTensor, Real>;
