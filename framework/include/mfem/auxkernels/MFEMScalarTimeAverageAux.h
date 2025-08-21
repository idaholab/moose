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
#include "MFEMAuxKernel.h"

/**
 * AuxKernel to compute a running time average of an MFEMVariable
 * using a linear blend.
 *
 *   avg_new(x) = (1 - w)*avg_old(x) + w*src(x), w = dt / (t - skip)
 */
class MFEMScalarTimeAverageAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMScalarTimeAverageAux(const InputParameters & parameters);

  virtual ~MFEMScalarTimeAverageAux() override = default;

  virtual void execute() override;

protected:
  /// Name of the source MFEMVariable name to take the average from
  const VariableName _source_var_name;
  /// Reference to the MFEMVariable underlying GridFunction
  const mfem::ParGridFunction & _source_var;
  /// Time before the averaging starts
  const mfem::real_t _skip;
};

#endif
