//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ValueThresholdMarker.h"
#include "FEProblem.h"
#include "MooseEnum.h"

registerMooseObject("MooseApp", ValueThresholdMarker);

InputParameters
ValueThresholdMarker::validParams()
{
  InputParameters params = QuadraturePointMarker::validParams();

  params.addParam<Real>("coarsen",
                        "The threshold value for coarsening.  Elements with variable "
                        "values beyond this will be marked for coarsening.");
  params.addParam<Real>("refine",
                        "The threshold value for refinement.  Elements with variable "
                        "values beyond this will be marked for refinement.");
  params.addClassDescription(
      "The refinement state based on a threshold value compared to the specified variable.");
  return params;
}

ValueThresholdMarker::ValueThresholdMarker(const InputParameters & parameters)
  : QuadraturePointMarker(parameters),
    _coarsen_set(parameters.isParamValid("coarsen")),
    _coarsen(parameters.get<Real>("coarsen")),
    _refine_set(parameters.isParamValid("refine")),
    _refine(parameters.get<Real>("refine")),
    _invert(parameters.get<bool>("invert"))
{
  if (_refine_set && _coarsen_set)
  {
    Real diff = _refine - _coarsen;
    if ((diff > 0 && _invert) || (diff < 0 && !_invert))
      mooseError("Invalid combination of refine, coarsen, and invert values specified");
  }
}

Marker::MarkerValue
ValueThresholdMarker::computeQpMarker()
{
  if (!_invert)
  {
    if (_refine_set && _u[_qp] > _refine)
      return REFINE;

    if (_coarsen_set && _u[_qp] < _coarsen)
      return COARSEN;
  }
  else
  {
    if (_refine_set && _u[_qp] < _refine)
      return REFINE;

    if (_coarsen_set && _u[_qp] > _coarsen)
      return COARSEN;
  }

  return _third_state;
}
