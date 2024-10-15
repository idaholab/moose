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

registerMooseObject("HeatTransferApp", GapFluxModelConduction);

InputParameters
GapFluxModelConduction::validParams()
{
  InputParameters params = GapFluxModelConductionBase::validParams();
  params.addClassDescription(
      "Gap flux model for varying gap conductance using a coupled variable for temperature");
  params.addRequiredCoupledVar("temperature", "The name of the temperature variable");
  params.addParam<FunctionName>(
      "gap_conductivity_function",
      "Thermal conductivity of the gap material as a function.  Multiplied "
      "by gap_conductivity.");
  params.addCoupledVar("gap_conductivity_function_variable",
                       "Variable to be used in the gap_conductivity_function in place of time");
  params.addParamNamesToGroup("gap_conductivity_function gap_conductivity_function_variable",
                              "Gap conductive flux");
  return params;
}

GapFluxModelConduction::GapFluxModelConduction(const InputParameters & parameters)
  : GapFluxModelConductionBase(parameters),
    _primary_T(adCoupledNeighborValue("temperature")),
    _secondary_T(adCoupledValue("temperature")),
    _gap_conductivity_function(isParamValid("gap_conductivity_function")
                                   ? &getFunction("gap_conductivity_function")
                                   : nullptr),
    _gap_conductivity_function_variable(isCoupled("gap_conductivity_function_variable")
                                            ? &coupledValue("gap_conductivity_function_variable")
                                            : nullptr)
{
}

ADReal
GapFluxModelConduction::computeFlux() const
{
  Real gap_conductivity_multiplier = 1;
  if (_gap_conductivity_function)
  {
    if (_gap_conductivity_function_variable)
      gap_conductivity_multiplier = _gap_conductivity_function->value(
          (*_gap_conductivity_function_variable)[_qp], _q_point[_qp]);
    else
      gap_conductivity_multiplier = _gap_conductivity_function->value(_t, _q_point[_qp]);
  }

  return computeConductionFlux(_secondary_T[_qp], _primary_T[_qp], gap_conductivity_multiplier);
}
