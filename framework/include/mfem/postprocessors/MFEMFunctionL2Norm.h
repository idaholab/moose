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

#include "MFEMNormPostprocessor.h"

class MFEMMesh;

/**
 * Compute the L2 norm of a scalar function over the mesh.
 */
class MFEMFunctionL2Norm : public MFEMNormPostprocessor
{
public:
  static InputParameters validParams();

  MFEMFunctionL2Norm(const InputParameters & parameters);

  /**
   * Get the L2 norm of the function.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  mfem::Coefficient & _coeff;
  MFEMMesh & _mesh;
};

#endif
