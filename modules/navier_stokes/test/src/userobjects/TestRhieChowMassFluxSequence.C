//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "TestRhieChowMassFluxSequence.h"
#include "MooseLinearVariableFV.h"
#include "RhieChowMassFlux.h"

registerMooseObject("NavierStokesTestApp", TestRhieChowMassFluxSequence);

InputParameters
TestRhieChowMassFluxSequence::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The Rhie-Chow user object.");
  params.addRequiredParam<MooseEnum>(
      "operation",
      MooseEnum("skip_face_flux_iteration zero_Ainv init_pressure_gradients"),
      "The reconstruction sequence to test.");
  params.addParam<VariableName>("pressure", "The pressure variable whose gradient is tested.");
  params.addParam<GradientMethodName>("gradient_method", "The additional gradient method to test.");
  return params;
}

TestRhieChowMassFluxSequence::TestRhieChowMassFluxSequence(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _rhie_chow(
        const_cast<RhieChowMassFlux &>(getUserObject<RhieChowMassFlux>("rhie_chow_user_object"))),
    _operation(getParam<MooseEnum>("operation")),
    _gradient_reader(nullptr)
{
  if (_operation == "init_pressure_gradients")
  {
    if (!isParamValid("pressure"))
      paramError("pressure", "This parameter is required for init_pressure_gradients.");
    if (!isParamValid("gradient_method"))
      paramError("gradient_method", "This parameter is required for init_pressure_gradients.");

    auto * const pressure = dynamic_cast<MooseLinearVariableFVReal *>(
        &_subproblem.getVariable(0, getParam<VariableName>("pressure")));
    if (!pressure)
      paramError("pressure", "The pressure must be a MooseLinearVariableFVReal.");

    _gradient_reader =
        &pressure->requestCellGradients(getParam<GradientMethodName>("gradient_method"));
  }
}

void
TestRhieChowMassFluxSequence::execute()
{
  if (_operation == "init_pressure_gradients")
  {
    mooseAssert(_gradient_reader, "The additional pressure gradient reader must be linked.");

    for (auto * const component : _gradient_reader->components())
    {
      component->zero();
      component->close();
    }

    _rhie_chow.initPressureGradient();

    bool gradient_is_zero = true;
    for (const auto * const component : _gradient_reader->components())
      gradient_is_zero = gradient_is_zero && component->l2_norm() == 0.0;

    if (gradient_is_zero)
      mooseError("RhieChowMassFlux::initPressureGradient() did not refresh the additional pressure "
                 "gradient reader.");
    return;
  }

  _rhie_chow.preparePISOCorrector();
  _rhie_chow.computeFaceMassFlux();

  if (_operation == "skip_face_flux_iteration")
    _rhie_chow.computeFaceMassFlux();
  else
    for (auto & Ainv : _rhie_chow.AinvComponents())
    {
      Ainv->zero();
      Ainv->close();
    }

  _rhie_chow.preparePressureRelaxation();
}
