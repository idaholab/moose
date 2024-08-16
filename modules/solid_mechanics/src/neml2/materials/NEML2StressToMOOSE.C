//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NEML2StressToMOOSE.h"
#include "ExecuteNEML2Model.h"

registerMooseObject("SolidMechanicsApp", NEML2StressToMOOSE);

#ifndef NEML2_ENABLED
NEML2ObjectStubImplementation(NEML2StressToMOOSE, Material);
#else

InputParameters
NEML2StressToMOOSE::validParams()
{
  auto params = ComputeLagrangianObjectiveStress::validParams();
  params.addClassDescription("Converts a stress output along with its Jacobian from a NEML2 model "
                             "to the correct stress formulation in MOOSE.");
  params.addRequiredParam<UserObjectName>("execute_neml2_model_uo",
                                          "User object managing the execution of the NEML2 model.");
  params.addRequiredParam<std::string>("neml2_stress_output", "NEML2 model stress output variable");
  params.addParam<std::string>("neml2_strain_input", "NEML2 model strain input variable");
  return params;
}

NEML2StressToMOOSE::NEML2StressToMOOSE(const InputParameters & params)
  : ComputeLagrangianObjectiveStress(params),
    _execute_neml2_model(getUserObject<ExecuteNEML2Model>("execute_neml2_model_uo")),
    _output_stress(_execute_neml2_model.getOutputView(
        neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_stress_output")))),
    _output_jacobian(_execute_neml2_model.getOutputDerivativeView(
        neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_stress_output")),
        neml2::utils::parse<neml2::VariableName>(getParam<std::string>("neml2_strain_input"))))
{
}

void
NEML2StressToMOOSE::computeProperties()
{
  if (!_execute_neml2_model.outputReady())
    return;

  // look up start index for current element
  _batch_index = _execute_neml2_model.getBatchIndex(_current_elem->id());

  ComputeLagrangianObjectiveStress::computeProperties();
}

void
NEML2StressToMOOSE::computeQpSmallStress()
{
  _small_stress[_qp] = NEML2Utils::toMOOSE<SymmetricRankTwoTensor>(
      _output_stress.batch_index({neml2::Size(_batch_index + _qp)}));
  _small_jacobian[_qp] = RankFourTensor(NEML2Utils::toMOOSE<SymmetricRankFourTensor>(
      _output_jacobian.batch_index({neml2::Size(_batch_index + _qp)})));
}

#endif
