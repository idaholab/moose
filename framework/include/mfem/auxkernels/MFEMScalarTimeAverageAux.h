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
 * avg_new(x) = (1 - w) * avg_old(x) + w * src(x), w = dt / (t - s), t > s
 */
class MFEMScalarTimeAverageAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMScalarTimeAverageAux(const InputParameters & parameters);

  virtual ~MFEMScalarTimeAverageAux() override = default;

  /// Computes the auxvariable.
  virtual void execute() override;

protected:
  /// Name of source MFEMVariable to take the time average of.
  const VariableName _source_var_name;
  /// Reference to source gridfunction coefficient.
  mfem::Coefficient & _source_var_coefficient;
  /// Reference to result gridfunction coefficient.
  mfem::Coefficient & _result_var_coefficient;
  /// Placeholder gridfunction to avoid read/write aliasing during projection.
  mfem::ParGridFunction _average_var;
  /// Time before the averaging starts.
  const Real & _skip;
  /// The current time.
  const Real & _time;
  /// Time step size.
  const Real & _dt;
};

#endif
