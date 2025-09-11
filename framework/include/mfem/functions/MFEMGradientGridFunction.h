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

#include "MFEMGeneralUserObject.h"

/**
 * Declares arbitrary parsed function of position, time, and any number of problem variables.
 */
class MFEMGradientGridFunction : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMGradientGridFunction(const InputParameters & parameters);
  virtual ~MFEMGradientGridFunction();

protected:
  /// grid function variable variable
  const VariableName & _var_name;
};

#endif