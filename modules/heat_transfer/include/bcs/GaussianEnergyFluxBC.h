//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Describes an incoming heat flux beam with a Gaussian profile. Form is taken from
 * https://en.wikipedia.org/wiki/Gaussian_beam
 */
class GaussianEnergyFluxBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();
  static InputParameters beamParams();

  GaussianEnergyFluxBC(const InputParameters & params);

  /**
   * Computes the beam flux given data from a flux object and the current integration point in the
   * domain, e.g. the location of a quadrature point in physical space for a finite element flux
   * object or the location of a face center for a finite volume flux object. The flux object should
   * have _x_beam_coord, _y_beam_coord, _z_beam_coord, _t, _P0, and _R data members
   */
  template <typename T, typename PointType>
  static ADReal beamFlux(const T & flux_obj, const PointType & flux_obj_location);

protected:
  virtual ADReal computeQpResidual() override;

  /// the total power of the beam
  const Real _P0;
  /// beam radius, specifically the radius at which the beam intensity falls to $1/e^2$ of its axial
  /// value
  const Real _R;
  /// the x-coordinate of the beam center
  const Function & _x_beam_coord;
  /// the y-coordinate of the beam center
  const Function & _y_beam_coord;
  /// the z-coordiinate of the beam center
  const Function & _z_beam_coord;
};

template <typename T, typename PointType>
ADReal
GaussianEnergyFluxBC::beamFlux(const T & flux_obj, const PointType & flux_obj_location)
{
  const Point origin(0, 0, 0);
  const RealVectorValue beam_coords{flux_obj._x_beam_coord.value(flux_obj._t, origin),
                                    flux_obj._y_beam_coord.value(flux_obj._t, origin),
                                    flux_obj._z_beam_coord.value(flux_obj._t, origin)};
  const auto r = (flux_obj_location - beam_coords).norm();
  const auto R2 = flux_obj._R * flux_obj._R;
  return -2 * flux_obj._P0 / (libMesh::pi * R2) * std::exp(-2 * r * r / R2);
}
