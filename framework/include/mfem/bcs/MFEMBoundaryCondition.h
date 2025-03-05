#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"
#include "Function.h"

namespace Moose::MFEM
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
  const VariableName & getTestVariableName() const { return _test_var_name; }

  mfem::Array<int> & getBoundaries() { return _bdr_markers; }

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName & _test_var_name;
  const std::vector<BoundaryName> & _boundary_names;
  mfem::Array<int> _bdr_attributes;
  mfem::Array<int> _bdr_markers;
};

#endif
