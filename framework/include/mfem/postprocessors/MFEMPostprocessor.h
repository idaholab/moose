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
#include "Postprocessor.h"
#include "MFEMGeneralUserObject.h"
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Postprocessor for MFEM results. Must inherit from Postprocessor in
 * order for MOOSE to call it.
 */
class MFEMPostprocessor : public MFEMGeneralUserObject, public Postprocessor
{
public:
  static InputParameters validParams();

  MFEMPostprocessor(const InputParameters & parameters);

  /**
   * This is called _after_ execute() and _after_ threadJoin()!  This is probably where you want to
   * do MPI communication!
   * Finalize is not required for Postprocessor implementations since work may be done in
   * getValue().
   */
  virtual void finalize() override {}
};

#endif
