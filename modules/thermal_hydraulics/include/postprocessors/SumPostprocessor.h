//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Computes a sum of postprocessor values
 *
 * TOOD: Generalize to take a vector of PPS names and possibly mode to MOOSE
 */
class SumPostprocessor : public GeneralPostprocessor
{
public:
  SumPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() const override;

protected:
  /// Postprocessors to add up
  std::vector<const PostprocessorValue *> _values;

public:
  static InputParameters validParams();
};
