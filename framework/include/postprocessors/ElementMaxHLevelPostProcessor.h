//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"

/**
 * This postprocessor computes the maximum element h-level over the whole domain.
 */
class ElementMaxHLevelPostProcessor : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  ElementMaxHLevelPostProcessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual void finalize() override;
  virtual Real getValue() const override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  unsigned int _max_level;
};
