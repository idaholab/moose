//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Computes mass float rate from specified uniform mass flux and cross-sectional area.
 * Reads mass flux value from postprocessor.
 */
class SCMMassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  SCMMassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// Mass flux provided by postprocessor
  const PostprocessorValue & _mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
};
