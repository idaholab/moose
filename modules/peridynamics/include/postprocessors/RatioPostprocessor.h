//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class RatioPostprocessor;

template <>
InputParameters validParams<RatioPostprocessor>();

/**
 * Postprocessor class to compute the ratio between two postprocessors
 * result = dividend / divisor
 */
class RatioPostprocessor : public GeneralPostprocessor
{
public:
  RatioPostprocessor(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override{};
  virtual PostprocessorValue getValue() override;

protected:
  /// Dividend from postprocessor 1
  const PostprocessorValue & _dividend;

  /// Divisor from postprocessor 2
  const PostprocessorValue & _divisor;
};
