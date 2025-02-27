#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"
#include "Function.h"

namespace MooseMFEM
{
class BCMap;
}

class MFEMBoundaryCondition : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMBoundaryCondition(const InputParameters & parameters);
  virtual ~MFEMBoundaryCondition() = default;

  // Get name of the test variable labelling the weak form this kernel is added to
  const std::string & getTestVariableName() const { return _test_var_name; }

protected:
  mfem::Array<int> GetMarkers(mfem::Mesh & mesh);
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  const std::string & _test_var_name;
  const std::vector<BoundaryName> & _boundary_names;
  mfem::Array<int> _bdr_attributes;

private:
  mfem::Array<int> _bdr_markers;

  friend class MooseMFEM::BCMap;
};

#endif
