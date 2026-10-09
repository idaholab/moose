//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelValue.h"
#include "KokkosDeformation.h"

class KokkosBodyForce : public Moose::Kokkos::KernelValue
{
public:
  static InputParameters validParams();

  KokkosBodyForce(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidualValue(const unsigned int qp,
                                                 AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpOffDiagJacobianValue(const unsigned int j,
                                                        const unsigned int jvar,
                                                        const unsigned int qp,
                                                        AssemblyDatum & datum) const;

protected:
  /// Scale factor
  const Moose::Kokkos::Scalar<const Real> _scale;

  /// Optional Postprocessor value
  const Moose::Kokkos::PostprocessorValue _postprocessor;

  /// Kinematics of the current configuration when the force is integrated over it
  const Moose::Kokkos::Deformation _deformation;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBodyForce::precomputeQpResidualValue(const unsigned int qp, AssemblyDatum & datum) const
{
  const Real force = -_scale * _postprocessor;

  return _deformation ? force * _deformation(datum, qp).J : force;
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosBodyForce::precomputeQpOffDiagJacobianValue(const unsigned int j,
                                                  const unsigned int /* jvar */,
                                                  const unsigned int qp,
                                                  AssemblyDatum & datum) const
{
  const auto c = _deformation.component(datum);

  if (c == libMesh::invalid_uint)
    return 0;

  return -_scale * _postprocessor *
         _deformation(datum, qp).dJ(c, _phi(datum, j, qp), _grad_phi(datum, j, qp));
}
