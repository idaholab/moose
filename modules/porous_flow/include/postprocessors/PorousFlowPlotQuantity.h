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

class PorousFlowSumQuantity;

/**
 * Extracts the value from PorousFlowSumQuantity userobject
 */
class PorousFlowPlotQuantity : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  PorousFlowPlotQuantity(const InputParameters & parameters);
  virtual ~PorousFlowPlotQuantity();

  virtual void initialize() override;
  virtual void execute() override;

  /// Returns the value of the PorousFlowSumQuantity
  virtual PostprocessorValue getValue() override;

protected:
  /// The PorousFlowSumQuantity userobject
  const PorousFlowSumQuantity & _total_mass;
};
