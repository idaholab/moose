//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Times.h"

/**
 * Simple times from an input parameter
 */
class DynamicInputTimes : public Times
{
public:
  static InputParameters validParams();
  DynamicInputTimes(const InputParameters & parameters);
  virtual ~DynamicInputTimes() = default;

protected:
  virtual void initialize() override;

private:
  /// The next external time sequences to hit
  const Real & _nexttime;
};
