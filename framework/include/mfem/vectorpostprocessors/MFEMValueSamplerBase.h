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

#include "MFEMSamplerBase.h"

/**
 * MFEM VectorPostprocessor base class for sampling a real-valued variable at a
 * set of points. Subclasses supply the point locations (e.g. MFEMPointValueSampler,
 * MFEMLineValueSampler).
 */
class MFEMValueSamplerBase : public MFEMSamplerBase
{
public:
  static InputParameters validParams();

  MFEMValueSamplerBase(const InputParameters & parameters, const std::vector<Point> & points);

  /// Interpolate the real variable at all query points.
  void execute() override;

protected:
  void finalizeValues() override;

private:
  const mfem::GridFunction & _var;
  mfem::Vector _interp_vals;
  std::vector<std::reference_wrapper<VectorPostprocessorValue>> _declared_vals;
};

#endif // MOOSE_MFEM_ENABLED
