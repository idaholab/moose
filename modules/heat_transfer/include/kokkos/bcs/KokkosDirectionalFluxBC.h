//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCValue.h"
#include "KokkosFunction.h"
#include "KokkosSelfShadowSideUserObject.h"

/**
 * Kokkos boundary condition applying a directional flux multiplied by the surface normal vector,
 * optionally accounting for self shadowing computed by a KokkosSelfShadowSideUserObject
 */
class KokkosDirectionalFluxBC : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosDirectionalFluxBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Radiation direction and magnitude vector
  const Moose::Kokkos::Real3 _direction;
  /// Function multiplying the flux
  const Moose::Kokkos::Function _func;
  /// Whether self shadowing is computed by a KokkosSelfShadowSideUserObject
  const bool _has_self_shadow;
  /// Illumination bitmasks of the local sides shared with the self shadowing user object
  Moose::Kokkos::Array2D<unsigned int> _illumination;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosDirectionalFluxBC::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  const Real projected_flux = -(_direction * datum.normals(qp));

  if (projected_flux <= 0)
    return 0;

  // tests if the bit at position qp is set
  if (_has_self_shadow && !(_illumination(datum.side(), datum.elemID()) & (1u << qp)))
    return 0;

  return -_func.value(_t, datum.q_point(qp)) * projected_flux;
}
