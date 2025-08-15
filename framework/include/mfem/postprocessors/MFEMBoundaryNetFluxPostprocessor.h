//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once
#include "MFEMPostprocessor.h"
#include "MFEMBoundaryRestrictable.h"

/**
 * Compute the total flux crossing a sideset in the problem.
 */
class MFEMBoundaryNetFluxPostprocessor : public MFEMPostprocessor, public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMBoundaryNetFluxPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  /**
   * Get the L2 Error.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  mfem::real_t _total_flux;
  mfem::GridFunction & _var;
};

#endif
