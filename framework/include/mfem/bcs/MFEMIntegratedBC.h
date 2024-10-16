#pragma once
#include "MFEMBoundaryCondition.h"

class MFEMIntegratedBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMIntegratedBC(const InputParameters & parameters);
  virtual ~MFEMIntegratedBC() = default;

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLinearFormIntegrator() = 0;

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBilinearFormIntegrator() = 0;

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }
};
