#ifdef MFEM_ENABLED

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
  virtual const std::string & getTrialVariableName() const override { return _var_dot_name; };

protected:
  // Name of variable (gridfunction) representing time derivative of variable.
  std::string _var_dot_name;
};

#endif
