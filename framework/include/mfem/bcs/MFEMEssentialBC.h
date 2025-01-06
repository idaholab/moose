#ifdef MFEM_ENABLED

#pragma once
#include "MFEMBoundaryCondition.h"

class MFEMEssentialBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMEssentialBC(const InputParameters & parameters);
  virtual ~MFEMEssentialBC() = default;

  // Get name of the trial variable (gridfunction) the kernel acts on.
  // Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }

  // Apply the essential BC, overwritign the values of gridfunc on the boundary as desired.
  virtual void ApplyBC(mfem::GridFunction & gridfunc, mfem::Mesh * mesh_) = 0;
};

#endif
