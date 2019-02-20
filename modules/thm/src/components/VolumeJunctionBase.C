#include "VolumeJunctionBase.h"

template <>
InputParameters
validParams<VolumeJunctionBase>()
{
  InputParameters params = validParams<FlowJunction>();

  params.addParam<bool>("compute_volume_from_areas",
                        false,
                        "Flag to compute junction volume from areas of connected flow channels");
  params.addParam<Real>("volume", "Volume of the junction [m^3]");
  params.addParam<std::vector<Real>>(
      "connected_areas", "Areas of connected flow channels, used for determining junction volume");

  params.addRequiredParam<Point>("position", "Spatial position of the center of the junction");

  return params;
}

VolumeJunctionBase::VolumeJunctionBase(const InputParameters & params)
  : FlowJunction(params),

    _volume(getParam<bool>("compute_volume_from_areas")
                ? computeVolumeFromAreas(getParam<std::vector<Real>>("connected_areas"))
                : getParam<Real>("volume")),

    _position(getParam<Point>("position"))
{
}

Real
VolumeJunctionBase::computeVolumeFromAreas(const std::vector<Real> & areas) const
{
  checkSizeEqualsNumberOfConnections<Real>("connected_areas");

  Real area_sum = 0.0;
  for (const auto & area : areas)
    area_sum += area;

  const Real junction_radius = std::sqrt(area_sum / (4.0 * libMesh::pi));
  return 4.0 / 3.0 * libMesh::pi * std::pow(junction_radius, 3);
}
