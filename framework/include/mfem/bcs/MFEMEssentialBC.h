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

#include "MFEMBoundaryCondition.h"

namespace Moose::MFEM
{
class EssentialBC : public BoundaryCondition
{
public:
  static InputParameters validParams();

  EssentialBC(const InputParameters & parameters);
  virtual ~EssentialBC() = default;

  /// Get name of the trial variable (gridfunction) the bc acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }

  // Apply the essential BC, overwriting the values of gridfunc on the boundary as desired.
  virtual void ApplyBC(mfem::GridFunction & gridfunc) = 0;
};

} // namespace Moose::MFEM
#endif
