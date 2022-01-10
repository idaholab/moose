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
#include "MortarInterface.h"

/**
 * Base class for creating new nodally-based mortar auxiliary kernels
 *
 */
template <typename ComputeValueType>
class MortarNodalAuxKernelTempl : public AuxKernel<ComputeValueType>, protected MortarInterface
{
public:
  static InputParameters validParams();

  MortarNodalAuxKernelTempl(const InputParameters & parameters);

  /**
   * Computes the value and stores it in the solution vector
   */
  void compute() override;

protected:
  void precalculateValue() override final;
};
