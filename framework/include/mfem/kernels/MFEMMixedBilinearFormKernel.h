#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

/*
Class to construct an MFEM mixed bilinear form integrator to apply to the equation system.
*/
class MFEMMixedBilinearFormKernel : public MFEMKernel<mfem::BilinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMMixedBilinearFormKernel(const InputParameters & parameters);
  ~MFEMMixedBilinearFormKernel() = default;

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const VariableName & getTrialVariableName() const override;

protected:
  // Name of the trial variable that the kernel is applied to.
  const VariableName _trial_var_name;
};

#endif
