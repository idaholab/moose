//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"
#include "KokkosFunction.h"

/**
 * Kokkos double ellipsoid volumetric heat source with a moving center given by functions of time
 */
class KokkosFunctionPathEllipsoidHeatSource : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosFunctionPathEllipsoidHeatSource(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /// power
  const Real _P;
  /// process efficienty
  const Real _eta;
  /// transverse ellipsoid axe
  const Real _rx;
  /// depth ellipsoid axe
  const Real _ry;
  /// longitudinal ellipsoid axe
  const Real _rz;
  /// scaling factor
  const Real _f;
  /// path of the heat source, x, y, z components
  const Moose::Kokkos::Function _function_x;
  const Moose::Kokkos::Function _function_y;
  const Moose::Kokkos::Function _function_z;

  Moose::Kokkos::MaterialProperty<Real> _volumetric_heat;
};

template <typename Derived>
KOKKOS_FUNCTION void
KokkosFunctionPathEllipsoidHeatSource::computeQpProperties(const unsigned int qp,
                                                           Datum & datum) const
{
  const auto p = datum.q_point(qp);
  const Moose::Kokkos::Real3 origin(0);

  // distance from the center of the heat source
  const Real dx = p(0) - _function_x.value(_t, origin);
  const Real dy = p(1) - _function_y.value(_t, origin);
  const Real dz = p(2) - _function_z.value(_t, origin);

  _volumetric_heat(datum, qp) =
      6.0 * ::Kokkos::sqrt(3.0) * _P * _eta * _f / (_rx * _ry * _rz * ::Kokkos::pow(M_PI, 1.5)) *
      ::Kokkos::exp(-(3.0 * dx * dx / (_rx * _rx) + 3.0 * dy * dy / (_ry * _ry) +
                      3.0 * dz * dz / (_rz * _rz)));
}
