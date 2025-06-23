//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUKernel.h"

template <typename Kernel>
class GPUBodyForce : public GPUKernel<Kernel>
{
  usingGPUKernelMembers(Kernel);

public:
  static InputParameters validParams()
  {
    InputParameters params = GPUKernel<Kernel>::validParams();
    params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
    params.addParam<PostprocessorName>(
        "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
    params.declareControllable("value");
    return params;
  }

  GPUBodyForce(const InputParameters & parameters)
    : GPUKernel<Kernel>(parameters),
      _scale(this->template getParam<Real>("value")),
      _postprocessor(getPostprocessorValue("postprocessor"))
  {
  }

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         ResidualDatum & datum) const
  {
    return -_test(datum, i, qp) * _scale * _postprocessor;
  }

protected:
  /// Scale factor
  GPUScalar<const Real> _scale;

  /// Optional Postprocessor value
  GPUPostprocessorValue _postprocessor;
};

class GPUBodyForceKernel final : public GPUBodyForce<GPUBodyForceKernel>
{
public:
  static InputParameters validParams();

  GPUBodyForceKernel(const InputParameters & parameters);
};
