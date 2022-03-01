//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimplePredictor.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", SimplePredictor);

InputParameters
SimplePredictor::validParams()
{
  InputParameters params = Predictor::validParams();
  params.addClassDescription(
      "Algorithm that will predict the next solution based on previous solutions.");
  return params;
}

SimplePredictor::SimplePredictor(const InputParameters & parameters) : Predictor(parameters) {}

bool
SimplePredictor::shouldApply()
{
  bool should_apply = true;
  should_apply = Predictor::shouldApply();

  if (_t_step < 2 || _dt_old <= 0)
    should_apply = false;

  if (!should_apply)
    _console << "  Skipping predictor this step" << std::endl;

  return should_apply;
}

void
SimplePredictor::apply(NumericVector<Number> & sln)
{
  _console << "  Applying predictor with scale factor = " << _scale << std::endl;

  Real dt_adjusted_scale_factor = _scale * _dt / _dt_old;
  if (dt_adjusted_scale_factor != 0.0)
  {
    sln *= (1.0 + dt_adjusted_scale_factor);
    sln.add(-dt_adjusted_scale_factor, _solution_older);
  }
}
