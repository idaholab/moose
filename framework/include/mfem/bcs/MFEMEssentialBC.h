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
#include "MFEMBoundaryCondition.h"

class MFEMEssentialBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMEssentialBC(const InputParameters & parameters);
  virtual ~MFEMEssentialBC() = default;

  /// Get name of the trial variable (gridfunction) the kernel acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }

  /// Apply the essential BC, overwritign the values of gridfunc on the boundary as desired.
  virtual void ApplyBC(mfem::GridFunction & gridfunc) = 0;
};

#endif
