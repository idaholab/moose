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

#include "MFEMPostprocessor.h"

class MFEMMesh;

/**
 * Compute the number of elements of the MFEM mesh over all processes.
 */
class MFEMNumElements : public MFEMPostprocessor
{
public:
  static InputParameters validParams();

  MFEMNumElements(const InputParameters & parameters);

  /**
   * Get the number of elements.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  MFEMMesh & _mesh;
};

#endif
