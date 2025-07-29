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

template <typename Derived>
class KokkosBodyForce : public Moose::Kokkos::Kernel<Derived>
{
  usingKokkosKernelMembers(Derived);

public:
  static InputParameters validParams();

  KokkosBodyForce(const InputParameters & parameters);

  KOKKOS_FUNCTION inline Real
  computeQpResidual(const unsigned int i, const unsigned int qp, ResidualDatum & datum) const;

protected:
  /// Scale factor
  const Moose::Kokkos::Scalar<const Real> _scale;

  /// Optional Postprocessor value
  const Moose::Kokkos::PostprocessorValue _postprocessor;
};

template <typename Derived>
InputParameters
KokkosBodyForce<Derived>::validParams()
{
  InputParameters params = Moose::Kokkos::Kernel<Derived>::validParams();
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  return params;
}

template <typename Derived>
KokkosBodyForce<Derived>::KokkosBodyForce(const InputParameters & parameters)
  : Moose::Kokkos::Kernel<Derived>(parameters),
    _scale(this->template getParam<Real>("value")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}

template <typename Derived>
KOKKOS_FUNCTION inline Real
KokkosBodyForce<Derived>::computeQpResidual(const unsigned int i,
                                            const unsigned int qp,
                                            ResidualDatum & datum) const
{
  return -_test(datum, i, qp) * _scale * _postprocessor;
}

class KokkosBodyForceKernel final : public KokkosBodyForce<KokkosBodyForceKernel>
{
public:
  static InputParameters validParams();

  KokkosBodyForceKernel(const InputParameters & parameters);
};
