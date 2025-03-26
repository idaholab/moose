//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThresholdElementSubdomainModifier.h"

class CoupledVarThresholdElementSubdomainModifier : public ThresholdElementSubdomainModifier
{
public:
  static InputParameters validParams();

  CoupledVarThresholdElementSubdomainModifier(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// The coupled variable used in the criterion
  const VariableValue & _v;
};
