//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVP1TemperatureSourceSink.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "HeatConductionNames.h"
#include "MathUtils.h"

registerMooseObject("HeatTransferApp", LinearFVP1TemperatureSourceSink);

InputParameters
LinearFVP1TemperatureSourceSink::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Implements the source and sink term for the temperature in the P1 radiation formulation.");
  params.addRequiredParam<MooseFunctorName>("G", "The incident radiation in the P1 model.");
  params.addParam<MooseFunctorName>(
      "absorption_coeff", 1.0, "The absorption coefficient of the material.");
  return params;
}

LinearFVP1TemperatureSourceSink::LinearFVP1TemperatureSourceSink(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _G(getFunctor<Real>("G")),
    _sigma_a(getFunctor<Real>("absorption_coeff"))
{
}

Real
LinearFVP1TemperatureSourceSink::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state_arg = determineState();

  return 4.0 * HeatConduction::Constants::sigma *
         std::pow(_var.getElemValue(*_current_elem_info, state_arg), 3) *
         _sigma_a(elem_arg, state_arg) * _current_elem_volume;
}

Real
LinearFVP1TemperatureSourceSink::computeRightHandSideContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state_arg = determineState();

  return _sigma_a(elem_arg, state_arg) * _G(elem_arg, state_arg) * _current_elem_volume;
}
