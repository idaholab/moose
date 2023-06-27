//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalStatistics.h"

class NodalVariableStatistics : public NodalStatistics
{
public:
  static InputParameters validParams();

  NodalVariableStatistics(const InputParameters & parameters);

private:
  virtual Real computeValue() override;

  /// The coupled variable used.
  const VariableValue & _v;
};
