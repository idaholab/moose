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
#include "MFEMGeneralUserObject.h"

/*
 * Compute the L2 error for a variable.
 */
class MFEML2Error : public MFEMPostprocessor
{
public:
  static InputParameters validParams();

  MFEML2Error(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  /**
   * Get the L2 Error.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  const VariableName & _var_name;
  const MFEMScalarCoefficientName & _coeff_name;
  mfem::Coefficient & _coeff;
  mfem::GridFunction & _var;
};

#endif
