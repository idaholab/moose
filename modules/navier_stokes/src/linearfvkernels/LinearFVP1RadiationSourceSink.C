//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVP1RadiationSourceSink.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "HeatConductionNames.h"
#include "MathUtils.h"

registerMooseObject("NavierStokesApp", LinearFVP1RadiationSourceSink);

InputParameters
LinearFVP1RadiationSourceSink::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Implements the source and sink term for the P1 radiation model solving for incident radiation G.");
  params.addRequiredParam<MooseFunctorName>("temperature_radiation", "The radiative temperature.");
  params.addParam<MooseFunctorName>(
      "absorption_coeff", 1.0, "The absorption coefficient of the material.");
  return params;
}

LinearFVP1RadiationSourceSink::LinearFVP1RadiationSourceSink(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _temperature_radiation(getFunctor<Real>("temperature_radiation")),
    _sigma_a(getFunctor<Real>("absorption_coeff"))
{
}

Real
LinearFVP1RadiationSourceSink::computeMatrixContribution()
{
  return _sigma_a(makeElemArg(_current_elem_info->elem()), determineState()) *
         _current_elem_volume;
}

Real
LinearFVP1RadiationSourceSink::computeRightHandSideContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state_arg = determineState();

  // The contribution to the right hand side is s_C*V_C
  return 4.0 * HeatConduction::Constants::sigma *
          _sigma_a(elem_arg,state_arg) *
          Utility::pow<4>(_temperature_radiation(elem_arg,state_arg)) * _current_elem_volume;
}
