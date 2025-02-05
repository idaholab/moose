//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralPostprocessor.h"

class LiftDragRewardPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  LiftDragRewardPostprocessor(const InputParameters & parameters);

  virtual void execute() override;
  virtual void initialize() override {}
  using Postprocessor::getValue;
  virtual Real getValue() const override;

protected:

  const PostprocessorValue & _lift;
  const PostprocessorValue & _drag;

  const unsigned int _averaging_window;

  const Real _coeff_1;
  const Real _coeff_2;

  Real _avg_lift;
  Real _avg_drag;

  std::vector<Real> _lift_history;
  std::vector<Real> _drag_history;

  unsigned int _replace_counter;

};
