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

#include "MFEMVariableSamplerBase.h"

/**
 * MFEM VectorPostprocessor base class for sampling a complex-valued variable at
 * a set of points. For each vector component of the variable, two
 * output columns are produced: "<var>_real_<i>" and "<var>_imag_<i>".
 *
 * Subclasses supply the point locations.
 */
class MFEMComplexVariableValueSamplerBase : public MFEMVariableSamplerBase
{
public:
  static InputParameters validParams();

  MFEMComplexVariableValueSamplerBase(const InputParameters & parameters,
                                      const std::vector<Point> & points);

  /// Interpolate the real and imaginary parts of the complex variable.
  void execute() override;

protected:
  void finalizeValues() override;

private:
  int getFESpaceContinuityType() const override;

  /// Complex grid function being sampled.
  const mfem::ParComplexGridFunction & _var;
  /// Values interpolated from the real part of the grid function.
  mfem::Vector _real_interp_vals;
  /// Values interpolated from the imaginary part of the grid function.
  mfem::Vector _imag_interp_vals;
  /// VectorPostprocessor output columns for the real components.
  std::vector<std::reference_wrapper<VectorPostprocessorValue>> _declared_real_vals;
  /// VectorPostprocessor output columns for the imaginary components.
  std::vector<std::reference_wrapper<VectorPostprocessorValue>> _declared_imag_vals;
};

#endif // MOOSE_MFEM_ENABLED
