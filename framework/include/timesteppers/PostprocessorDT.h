//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeStepper.h"
#include "PostprocessorInterface.h"

/**
 * Computes the value of dt based on a postprocessor value
 */
class PostprocessorDT : public TimeStepper, public PostprocessorInterface
{
public:
  static InputParameters validParams();

  PostprocessorDT(const InputParameters & parameters);

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  const PostprocessorValue & _pps_value;
  bool _has_initial_dt;
  Real _initial_dt;

  /// Multiplier applied to the postprocessor value
  const Real & _scale;

  /// Offset added to the postprocessor value
  const Real & _offset;
};
