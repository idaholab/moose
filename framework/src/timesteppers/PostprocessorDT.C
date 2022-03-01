//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorDT.h"

registerMooseObject("MooseApp", PostprocessorDT);

InputParameters
PostprocessorDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addClassDescription("Computes timestep based on a Postprocessor value.");
  params.addRequiredParam<PostprocessorName>("postprocessor",
                                             "The name of the postprocessor that computes the dt");
  params.addParam<Real>("dt", "Initial value of dt");
  params.addParam<Real>("scale", 1, "Multiple scale and supplied postprocessor value.");
  params.addParam<Real>("offset", 0, "Add an offset to the supplied postprocessor value.");
  params.addDeprecatedParam<Real>("factor",
                                  "Add an offset to the supplied postprocessor value",
                                  "offset has replaced factor for that same purpose");
  return params;
}

PostprocessorDT::PostprocessorDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    PostprocessorInterface(this),
    _pps_value(getPostprocessorValue("postprocessor")),
    _has_initial_dt(isParamValid("dt")),
    _initial_dt(_has_initial_dt ? getParam<Real>("dt") : 0.),
    _scale(getParam<Real>("scale")),
    _offset(isParamValid("offset") ? getParam<Real>("offset") : getParam<Real>("factor"))
{
}

Real
PostprocessorDT::computeInitialDT()
{
  if (_has_initial_dt)
    return _initial_dt;
  else
    return computeDT();
}

Real
PostprocessorDT::computeDT()
{
  return _scale * _pps_value + _offset;
}
