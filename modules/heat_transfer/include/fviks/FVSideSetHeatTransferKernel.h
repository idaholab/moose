//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVInterfaceKernel.h"

/**
 * FV interface kernel for modeling heat transfer across an internal sideset
 * using an interface conductance.
 */
class FVSideSetHeatTransferKernel : public FVInterfaceKernel
{
public:
  static InputParameters validParams();

  FVSideSetHeatTransferKernel(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// Thermal conductance across the interface
  const Moose::Functor<ADReal> & _conductance;
};
