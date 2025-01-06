#ifdef MFEM_ENABLED

#pragma once
#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"
#include "Function.h"

class MFEMBoundaryCondition : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMBoundaryCondition(const InputParameters & parameters);
  virtual ~MFEMBoundaryCondition() = default;

  // Get name of the test variable labelling the weak form this kernel is added to
  const std::string & getTestVariableName() const { return _test_var_name; }

  mfem::Array<int> GetMarkers(mfem::Mesh & mesh);
  mfem::Array<int> _bdr_markers;

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  std::string _test_var_name;
  std::vector<BoundaryName> _boundary_names;
  mfem::Array<int> _bdr_attributes;
};

#endif
