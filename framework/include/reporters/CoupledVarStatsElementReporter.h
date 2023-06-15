//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementStatsReporter.h"

class CoupledVarStatsElementReporter : public ElementStatsReporter
{
public:
  static InputParameters validParams();

  CoupledVarStatsElementReporter(const InputParameters & parameters);

private:
  /// The coupled variable used.
  const VariableValue & _v;
  virtual Real computeValue();
};
