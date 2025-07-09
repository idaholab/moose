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

/**
 * Class to construct an auxiliary solver used to update an auxvariable.
 */
class MFEMAuxKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMAuxKernel(const InputParameters & parameters);
  virtual ~MFEMAuxKernel() = default;

  // Method called to update any owned objects upon a mesh update.
  virtual void update(){};

protected:
  /// Name of auxvariable to store the result of the auxkernel in.
  const AuxVariableName _result_var_name;

  /// Reference to result gridfunction.
  mfem::ParGridFunction & _result_var;
};

#endif
