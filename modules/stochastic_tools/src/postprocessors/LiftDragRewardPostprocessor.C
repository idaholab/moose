//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LiftDragRewardPostprocessor.h"

#include <cmath>
#include <numeric>

registerMooseObject("StochasticToolsApp", LiftDragRewardPostprocessor);

InputParameters
LiftDragRewardPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<PostprocessorName>(
      "lift", "Postprocessor that supplies the current lift coefficient.");
  params.addRequiredParam<PostprocessorName>(
      "drag", "Postprocessor that supplies the current drag coefficient.");

  params.addParam<unsigned int>(
      "averaging_window",
      1,
      "Number of timesteps to include in the rolling lift and drag averages.");
  params.addParam<Real>(
      "coeff_1", 1.59, "Baseline reward offset before drag and lift penalties are applied.");
  params.addParam<Real>("coeff_2", 0.2, "Multiplier applied to the absolute-value lift penalty.");

  params.addClassDescription(
      "Turns rolling lift and drag coefficients into a simple scalar reward signal.");

  return params;
}

LiftDragRewardPostprocessor::LiftDragRewardPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _lift(getPostprocessorValue("lift")),
    _drag(getPostprocessorValue("drag")),
    _averaging_window(getParam<unsigned int>("averaging_window")),
    _coeff_1(getParam<Real>("coeff_1")),
    _coeff_2(getParam<Real>("coeff_2")),
    _avg_lift(0.0),
    _avg_drag(0.0),
    _lift_history(std::vector<Real>(_averaging_window, 0.0)),
    _drag_history(std::vector<Real>(_averaging_window, 0.0))
{
}

Real
LiftDragRewardPostprocessor::getValue() const
{
  return _coeff_1 - _avg_drag - _coeff_2 * std::abs(_avg_lift);
}

void
LiftDragRewardPostprocessor::execute()
{
  auto rolling_index = _t_step % _averaging_window;
  _lift_history[rolling_index] = _lift;
  _drag_history[rolling_index] = _drag;

  if (!rolling_index)
  {
    const auto normalization = _t_step ? _averaging_window : 1;
    _avg_lift = std::reduce(_lift_history.begin(), _lift_history.end()) / normalization;
    _avg_drag = std::reduce(_drag_history.begin(), _drag_history.end()) / normalization;
    _lift_history = std::vector<Real>(_averaging_window, 0.0);
    _drag_history = std::vector<Real>(_averaging_window, 0.0);
  }

  _replace_counter++;
}
