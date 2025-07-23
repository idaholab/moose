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
#include "MFEMMassKernel.h"

/*
 * \f[
 * (\beta du/dt, u')
 * \f]
 */
class MFEMTimeDerivativeMassKernel : public MFEMMassKernel
{
public:
  static InputParameters validParams();

  MFEMTimeDerivativeMassKernel(const InputParameters & parameters);

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const VariableName & getTrialVariableName() const override { return _var_dot_name; };

protected:
  // Name of variable (gridfunction) representing time derivative of variable.
  const VariableName _var_dot_name;
};

#endif
