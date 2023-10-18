//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelRadiationBase.h"

InputParameters
GapFluxModelRadiationBase::validParams()
{
  InputParameters params = GapFluxModelBase::validParams();
  params.addParam<Real>("stefan_boltzmann", 5.670373e-8, "Stefan-Boltzmann constant");
  params.addRangeCheckedParam<Real>("primary_emissivity",
                                    1,
                                    "primary_emissivity>=0 & primary_emissivity<=1",
                                    "The emissivity of the primary surface");
  params.addRangeCheckedParam<Real>("secondary_emissivity",
                                    1,
                                    "secondary_emissivity>=0 & secondary_emissivity<=1",
                                    "The emissivity of the secondary surface");
  return params;
}

GapFluxModelRadiationBase::GapFluxModelRadiationBase(const InputParameters & parameters)
  : GapFluxModelBase(parameters),
    _stefan_boltzmann(getParam<Real>("stefan_boltzmann")),
    _radial_coord(),
    _eps_primary(getParam<Real>("primary_emissivity")),
    _eps_secondary(getParam<Real>("secondary_emissivity")),
    _has_zero_emissivity(_eps_primary == 0 && _eps_secondary == 0),
    _parallel_plate_emissivity(
        _has_zero_emissivity ? 0 : 1 / (1.0 / _eps_primary + 1.0 / _eps_secondary - 1))
{
  const auto & coord_systems = _mesh.getCoordSystem();
  mooseAssert(coord_systems.size(), "This better not be empty");
  _coord_system = coord_systems.begin()->second;
  for (const auto [sub_id, coord_system_type] : coord_systems)
  {
    libmesh_ignore(sub_id);
    if (coord_system_type != _coord_system)
      mooseError(
          "Multiple coordinate system types detected. If you need this object to work with "
          "multiple coordinate system types in the same mesh, please contact a MOOSE developer");
  }

  if (_coord_sys == Moose::COORD_RZ)
    _radial_coord = _mesh.getAxisymmetricRadialCoord();
}

Real
GapFluxModelRadiationBase::emissivity() const
{
  if (_has_zero_emissivity)
    return 0;

  switch (_coord_system)
  {
    case Moose::COORD_XYZ:
      return _parallel_plate_emissivity;

    case Moose::COORD_RZ:
    {
      const auto primary_r = _primary_point.point(_radial_coord);
      const auto secondary_r = _secondary_point.point(_radial_coord);
      const bool primary_is_inner = primary_r < secondary_r;
      const auto inner_r = primary_is_inner ? primary_r : secondary_r;
      const auto outer_r = primary_is_inner ? secondary_r : primary_r;
      const auto inner_eps = primary_is_inner ? _eps_primary : _eps_secondary;
      const auto outer_eps = primary_is_inner ? _eps_secondary : _eps_primary;

      // Taken from our documentation of FVInfiniteCylinderRadiativeBC
      return inner_eps * outer_eps * outer_r /
             (outer_eps * outer_r + inner_eps * inner_r * (1 - outer_eps));
    }

    default:
      mooseError("spherical coordinates not yet supported for this object");
  }
}

ADReal
GapFluxModelRadiationBase::computeRadiationFlux(const ADReal & secondary_T,
                                                const ADReal & primary_T) const
{
  // We add 'surface_integration_factor' to account for the surface integration of the conductance
  // due to radiation.
  const ADReal temp_func =
      (primary_T * primary_T + secondary_T * secondary_T) * (primary_T + secondary_T);

  return (primary_T - secondary_T) * _stefan_boltzmann * temp_func * emissivity() *
         _surface_integration_factor;
}
