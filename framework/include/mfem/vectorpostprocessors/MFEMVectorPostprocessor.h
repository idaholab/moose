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
#include "VectorPostprocessor.h"

/*
 * Vector postprocessor for MFEM results. Must inherit from VectorPostprocessor
 * in order for MOOSE to call it.
 */
class MFEMVectorPostprocessor : public MFEMGeneralUserObject, public VectorPostprocessor
{
public:
  static InputParameters validParams();

  MFEMVectorPostprocessor(const InputParameters & parameters);
};

#endif // MOOSE_MFEM_ENABLED
