//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubcellInterfacialTipPosition.h"

#include "FaceInfo.h"
#include "MooseMesh.h"
#include "metaphysicl/raw_type.h"

#include <algorithm>
#include <limits>

registerMooseObject("NavierStokesApp", SubcellInterfacialTipPosition);

namespace
{
Real
clampTipVolumeFraction(const Real value)
{
  return std::max(0.0, std::min(1.0, value));
}
}

InputParameters
SubcellInterfacialTipPosition::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum coordinate("x=0 y=1 z=2", "x");
  MooseEnum extremum_type("max min", "max");
  MooseEnum reported_extremum_type("max min", "max");
  params.addRequiredParam<MooseFunctorName>(
      "volume_fraction", "The volume-fraction functor used to reconstruct the interface point.");
  params.addParam<MooseEnum>(
      "tip_direction",
      coordinate,
      "Coordinate direction used to define the extremal interface tip point.");
  params.addParam<MooseEnum>(
      "reported_component",
      coordinate,
      "Coordinate component of the extremal tip point to return.");
  params.addParam<MooseEnum>(
      "extremum_type", extremum_type, "Whether the tip is the maximum or minimum in tip_direction.");
  params.addParam<MooseEnum>("reported_extremum_type",
                             reported_extremum_type,
                             "When tip_band_width is positive and reported_component differs from "
                             "tip_direction, choose the maximum or minimum reported component among "
                             "interface points inside that band.");
  params.addParam<Real>("threshold", 0.5, "The volume-fraction threshold that defines the interface.");
  params.addRangeCheckedParam<Real>(
      "minimum_alignment",
      0.5,
      "minimum_alignment>=0 & minimum_alignment<=1",
      "Minimum absolute projection of the face normal onto the tip direction.");
  params.addRangeCheckedParam<Real>(
      "tip_band_width",
      0.0,
      "tip_band_width>=0",
      "Optional band width behind the extremal tip. If positive and reported_component differs "
      "from tip_direction, the reported value is taken from interface points within this band.");
  params.addParam<Real>("value_if_no_interface",
                        0.0,
                        "Value returned if no threshold crossing is found.");
  params.addClassDescription(
      "Computes a coordinate component of the extremal subcell interface tip point from "
      "neighboring FV cell-centered volume fractions.");
  return params;
}

SubcellInterfacialTipPosition::SubcellInterfacialTipPosition(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _volume_fraction(getFunctor<Real>("volume_fraction")),
    _tip_direction(static_cast<unsigned int>(getParam<MooseEnum>("tip_direction"))),
    _reported_component(static_cast<unsigned int>(getParam<MooseEnum>("reported_component"))),
    _search_max(getParam<MooseEnum>("extremum_type") == "max"),
    _search_reported_max(getParam<MooseEnum>("reported_extremum_type") == "max"),
    _threshold(getParam<Real>("threshold")),
    _minimum_alignment(getParam<Real>("minimum_alignment")),
    _tip_band_width(getParam<Real>("tip_band_width")),
    _value_if_no_interface(getParam<Real>("value_if_no_interface")),
    _tip_point(),
    _reported_value(_value_if_no_interface),
    _found_candidate(false)
{
  if (_tip_direction >= _mesh.dimension())
    paramError("tip_direction",
               "Requested tip_direction is incompatible with mesh dimension ",
               _mesh.dimension(),
               ".");

  if (_reported_component >= _mesh.dimension())
    paramError("reported_component",
               "Requested reported_component is incompatible with mesh dimension ",
               _mesh.dimension(),
               ".");
}

void
SubcellInterfacialTipPosition::initialize()
{
  const Real init_value = _search_max ? -std::numeric_limits<Real>::max() : std::numeric_limits<Real>::max();
  for (const auto i : make_range(LIBMESH_DIM))
    _tip_point(i) = init_value;
  _reported_value = _search_reported_max ? -std::numeric_limits<Real>::max()
                                         : std::numeric_limits<Real>::max();
  _found_candidate = false;
  _crossing_points.clear();
}

void
SubcellInterfacialTipPosition::updateCandidate(const Point & point)
{
  if (!_found_candidate)
  {
    _tip_point = point;
    _found_candidate = true;
    return;
  }

  if (_search_max)
  {
    if (point(_tip_direction) > _tip_point(_tip_direction))
      _tip_point = point;
  }
  else if (point(_tip_direction) < _tip_point(_tip_direction))
    _tip_point = point;
}

bool
SubcellInterfacialTipPosition::pointIsWithinTipBand(const Point & point) const
{
  if (_tip_band_width <= 0.0 || _reported_component == _tip_direction)
    return std::abs(point(_tip_direction) - _tip_point(_tip_direction)) <= libMesh::TOLERANCE;

  if (_search_max)
    return point(_tip_direction) >= _tip_point(_tip_direction) - _tip_band_width - libMesh::TOLERANCE;

  return point(_tip_direction) <= _tip_point(_tip_direction) + _tip_band_width + libMesh::TOLERANCE;
}

void
SubcellInterfacialTipPosition::execute()
{
  const auto time_arg = determineState();

  for (const auto * const fi : _mesh.faceInfo())
  {
    if (!fi || !fi->elemPtr() || !fi->neighborPtr())
      continue;

    const auto normal = fi->normal();
    const Real alignment = std::abs(normal(_tip_direction)) / std::max(normal.norm(), libMesh::TOLERANCE);
    if (alignment < _minimum_alignment)
      continue;

    const Point elem_centroid = fi->elemCentroid();
    const Point neighbor_centroid = fi->neighborCentroid();

    const Real elem_alpha =
        clampTipVolumeFraction(MetaPhysicL::raw_value(_volume_fraction(makeElemArg(fi->elemPtr()), time_arg)));
    const Real neighbor_alpha = clampTipVolumeFraction(
        MetaPhysicL::raw_value(_volume_fraction(makeElemArg(fi->neighborPtr()), time_arg)));

    const Real delta_alpha = neighbor_alpha - elem_alpha;
    if (std::abs(delta_alpha) <= libMesh::TOLERANCE)
      continue;

    const Real signed_elem = elem_alpha - _threshold;
    const Real signed_neighbor = neighbor_alpha - _threshold;
    if (signed_elem * signed_neighbor > 0.0)
      continue;

    const Real interpolation = (_threshold - elem_alpha) / delta_alpha;
    if (interpolation < -libMesh::TOLERANCE || interpolation > 1.0 + libMesh::TOLERANCE)
      continue;

    Point crossing_point = elem_centroid + interpolation * (neighbor_centroid - elem_centroid);
    _crossing_points.push_back(crossing_point);
    updateCandidate(crossing_point);
  }
}

void
SubcellInterfacialTipPosition::finalize()
{
  if (_search_max)
    gatherMax(_tip_point(_tip_direction));
  else
    gatherMin(_tip_point(_tip_direction));

  bool found_candidate = _found_candidate;
  _communicator.max(found_candidate);
  _found_candidate = found_candidate;

  if (!_found_candidate)
  {
    _reported_value = _value_if_no_interface;
    return;
  }

  if (_reported_component == _tip_direction)
  {
    _reported_value = _tip_point(_tip_direction);
    return;
  }

  Real local_reported_value =
      _search_reported_max ? -std::numeric_limits<Real>::max() : std::numeric_limits<Real>::max();
  bool found_reported_candidate = false;

  for (const auto & point : _crossing_points)
  {
    if (!pointIsWithinTipBand(point))
      continue;

    if (!found_reported_candidate)
    {
      local_reported_value = point(_reported_component);
      found_reported_candidate = true;
      continue;
    }

    if (_search_reported_max)
      local_reported_value = std::max(local_reported_value, point(_reported_component));
    else
      local_reported_value = std::min(local_reported_value, point(_reported_component));
  }

  if (_search_reported_max)
    gatherMax(local_reported_value);
  else
    gatherMin(local_reported_value);

  bool found_any_reported_candidate = found_reported_candidate;
  _communicator.max(found_any_reported_candidate);
  _reported_value = found_any_reported_candidate ? local_reported_value : _value_if_no_interface;
}

Real
SubcellInterfacialTipPosition::getValue() const
{
  return _reported_value;
}
