//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LiftDragRewardPostprocessor.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "MathUtils.h"
#include "TransientBase.h"
#include "Restartable.h"
#include "libmesh/enum_norm_type.h"

registerMooseObject("MooseApp", LiftDragRewardPostprocessor);

InputParameters
LiftDragRewardPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<PostprocessorName>("lift","Lift coeff");
  params.addRequiredParam<PostprocessorName>("drag","Drag coeff");

  params.addParam<unsigned int>("averaging_window", 1, "The window");
  params.addParam<Real>("coeff_1", 1.59, "Coeff 1");
  params.addParam<Real>("coeff_2", 0.2, "Coeff 2");

  params.addClassDescription("Blabla.");

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
    _lift_history(std::vector<Real>(_averaging_window,0.0)),
    _drag_history(std::vector<Real>(_averaging_window,0.0))
{
}

Real
LiftDragRewardPostprocessor::getValue() const
{
  // std::cout << "Reward" << _coeff_1 - _avg_drag - _coeff_2*std::abs(_avg_lift) << std::endl;
  return _coeff_1 - _avg_drag - _coeff_2*std::abs(_avg_lift);
}

void
LiftDragRewardPostprocessor::execute()
{
  auto rolling_index = _t_step % _averaging_window;
  // std::cout << " Rolling index " << rolling_index << std::endl;

  // std::cout << "Lift" << _lift << " drag " << _drag << std::endl;
  _lift_history[rolling_index] = _lift;
  _drag_history[rolling_index] = _drag;

  // std::cout << Moose::stringify(_lift_history) << std::endl;
  // std::cout << Moose::stringify(_drag_history) << std::endl;

  if (!rolling_index)
  {
    const auto normalization = _t_step ? _averaging_window : 1;
    // std::cout << Moose::stringify(_lift_history) << std::endl;
    // std::cout << Moose::stringify(_drag_history) << std::endl;
    _avg_lift = std::reduce(_lift_history.begin(), _lift_history.end())/normalization;
    _avg_drag = std::reduce(_drag_history.begin(), _drag_history.end())/normalization;
    _lift_history = std::vector<Real>(_averaging_window,0.0);
    _drag_history = std::vector<Real>(_averaging_window,0.0);
  }

  _replace_counter++;
}
