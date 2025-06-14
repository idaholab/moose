//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

  bool isBoundaryRestricted() const
  {
    return !(_bdr_attributes.Size() == 1 && _bdr_attributes[0] == -1);
  }

  mfem::Array<int> & getBoundaries() { return _bdr_markers; }

protected:
  // Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName & _test_var_name;
  const std::vector<BoundaryName> & _boundary_names;
  mfem::Array<int> _bdr_attributes;
  mfem::Array<int> _bdr_markers;
};

#endif
