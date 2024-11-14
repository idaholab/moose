//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVVolumetricHeatTransfer.h"
#include "SubProblem.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVVolumetricHeatTransfer);

InputParameters
LinearFVVolumetricHeatTransfer::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Represents a heat transfer term between the fluid and a homogenized structure.");
  params.addRequiredParam<MooseFunctorName>("h_solid_fluid",
                                            "Name of the convective heat transfer coefficient.");
  params.addRequiredParam<bool>("is_solid", "Whether this kernel acts on the solid temperature.");
  params.addRequiredParam<VariableName>(NS::T_fluid, "Fluid temperature.");
  params.addRequiredParam<VariableName>(NS::T_solid, "Solid temperature.");
  return params;
}

LinearFVVolumetricHeatTransfer::LinearFVVolumetricHeatTransfer(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _h_solid_fluid(getFunctor<Real>("h_solid_fluid")),
    _temp_fluid(getTemperatureVariable(NS::T_fluid)),
    _temp_solid(getTemperatureVariable(NS::T_solid)),
    _is_solid(getParam<bool>("is_solid"))
{
}

const MooseLinearVariableFV<Real> &
LinearFVVolumetricHeatTransfer::getTemperatureVariable(const std::string & vname)
{
  auto * ptr = dynamic_cast<MooseLinearVariableFV<Real> *>(
      &_fe_problem.getVariable(_tid, getParam<VariableName>(vname)));

  if (!ptr)
    paramError(
        vname, "The variable supplied to ", vname, " should be of type MooseLinearVariableFVReal!");

  return *ptr;
}

Real
LinearFVVolumetricHeatTransfer::computeMatrixContribution()
{
  const auto elem = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();

  return _h_solid_fluid(elem, state) * _current_elem_volume;
}

Real
LinearFVVolumetricHeatTransfer::computeRightHandSideContribution()
{
  const auto elem = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();

  const auto temp_value = _is_solid ? _temp_fluid.getElemValue(*_current_elem_info, state)
                                    : _temp_solid.getElemValue(*_current_elem_info, state);

  return _h_solid_fluid(elem, state) * temp_value * _current_elem_volume;
}
