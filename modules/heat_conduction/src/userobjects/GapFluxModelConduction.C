//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelConduction.h"
#include "libmesh/utility.h"
#include "Function.h"

registerMooseObject("HeatConductionApp", GapFluxModelConduction);

InputParameters
GapFluxModelConduction::validParams()
{
  InputParameters params = GapFluxModelBase::validParams();
  params.addClassDescription("Gap flux model for varying gap conductance");
  params.addRequiredCoupledVar("temperature", "The name of the temperature variable");
  params.addParam<Real>("gap_conductivity", 1.0, "Gap conductivity value");
  params.addParam<FunctionName>(
      "gap_conductivity_function",
      "Thermal conductivity of the gap material as a function.  Multiplied "
      "by gap_conductivity.");
  params.addCoupledVar("gap_conductivity_function_variable",
                       "Variable to be used in the gap_conductivity_function in place of time");
  params.addRangeCheckedParam<Real>(
      "min_gap", 1e-6, "min_gap>0", "A minimum gap (denominator) size");
  params.addRangeCheckedParam<unsigned int>(
      "min_gap_order",
      0,
      "min_gap_order<=1",
      "Order of the Taylor expansion below min_gap for GapFluxModelConduction");

  return params;
}

GapFluxModelConduction::GapFluxModelConduction(const InputParameters & parameters)
  : GapFluxModelBase(parameters),
    _primary_T(adCoupledNeighborValue("temperature")),
    _secondary_T(adCoupledValue("temperature")),
    _gap_conductivity(getParam<Real>("gap_conductivity")),
    _gap_conductivity_function(isParamValid("gap_conductivity_function")
                                   ? &getFunction("gap_conductivity_function")
                                   : nullptr),
    _gap_conductivity_function_variable(isCoupled("gap_conductivity_function_variable")
                                            ? &coupledValue("gap_conductivity_function_variable")
                                            : nullptr),
    _min_gap(getParam<Real>("min_gap")),
    _min_gap_order(getParam<unsigned int>("min_gap_order"))

{
}

ADReal
GapFluxModelConduction::computeFlux() const
{
  ADReal gap_conductivity = _gap_conductivity;

  if (_gap_conductivity_function)
  {
    if (_gap_conductivity_function_variable)
      gap_conductivity *= _gap_conductivity_function->value(
          (*_gap_conductivity_function_variable)[_qp], _q_point[_qp]);
    else
      gap_conductivity *= _gap_conductivity_function->value(_t, _q_point[_qp]);
  }

  return (_primary_T[_qp] - _secondary_T[_qp]) * gap_conductivity * gapAttenuation();
}

ADReal
GapFluxModelConduction::gapAttenuation() const
{

  mooseAssert(_min_gap > 0, "min_gap must be larger than zero.");

  if (_adjusted_length > _min_gap)
  {
    return 1.0 / _adjusted_length;
  }
  else
    switch (_min_gap_order)
    {
      case 0:
        return 1.0 / _min_gap;

      case 1:
        return 1.0 / _min_gap - (_adjusted_length - _min_gap) / (_min_gap * _min_gap);

      default:
        mooseError("Invalid Taylor expansion order for gap attenuation in GapFluxModelConduction");
    }
}
