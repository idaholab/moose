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

/**
 * Base class used to set the initial value(s) on an MFEMVariable.
 */
#include "MFEMGeneralUserObject.h"

class MFEMInitialCondition : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();
  MFEMInitialCondition(const InputParameters & params);
};

#endif
