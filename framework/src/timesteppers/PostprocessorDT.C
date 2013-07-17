/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PostprocessorDT.h"

template<>
InputParameters validParams<PostprocessorDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor that computes the dt");
  params.addParam<Real>("dt", "Initial value of dt");
  return params;
}

PostprocessorDT::PostprocessorDT(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters),
    PostprocessorInterface(parameters),
    _pps_value(getPostprocessorValue("postprocessor")),
    _has_initial_dt(isParamValid("dt")),
    _initial_dt(_has_initial_dt ? getParam<Real>("dt") : 0.)
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
  return _pps_value;
}
