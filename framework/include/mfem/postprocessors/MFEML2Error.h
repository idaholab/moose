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

namespace Moose::MFEM
{
/**
 * Compute the L2 error for a variable.
 */
class L2Error : public Postprocessor
{
public:
  static InputParameters validParams();

  L2Error(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  /**
   * Get the L2 Error.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  mfem::Coefficient & _coeff;
  mfem::GridFunction & _var;
};

} // namespace Moose::MFEM
#endif
