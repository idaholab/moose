//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 *  ChannelGradientVectorPostprocessor is a VectorPostprocessor that calculates
 *  the difference between two LineValueSampler vector postprocessors.
 */

class ChannelGradientVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  ChannelGradientVectorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  /// The line value sampler names
  VectorPostprocessorName _lv1_name;
  VectorPostprocessorName _lv2_name;

  /**
   * The axis the line value samplers sampled along; this vpps will not work
   * if the line value samplers varied more than one coordinate
   */
  std::string _axis;

  /// The variable values from the line value samplers
  const VectorPostprocessorValue & _lv1_variable_values;
  const VectorPostprocessorValue & _lv2_variable_values;

  /// The axis coordinate values for the line value samplers
  const VectorPostprocessorValue & _lv1_axis_values;

  /// Axis coordinate values; corresponds to values from _lv1_axis_values
  VectorPostprocessorValue * _axis_values;

  /// The vector holding the difference between the two line value samplers
  VectorPostprocessorValue * _gradient_values;
};
