//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubcellInterfacialPosition.h"

#include "FaceInfo.h"
#include "MooseMesh.h"
#include "metaphysicl/raw_type.h"

#include <algorithm>
#include <limits>

registerMooseObject("NavierStokesApp", SubcellInterfacialPosition);

namespace
{
Real
clampVolumeFraction(const Real value)
{
  return std::max(0.0, std::min(1.0, value));
}
}

InputParameters
SubcellInterfacialPosition::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum direction("x=0 y=1 z=2", "x");
  MooseEnum extremum_type("max min", "max");
  params.addRequiredParam<MooseFunctorName>(
      "volume_fraction", "The volume-fraction functor used to reconstruct the interface position.");
  params.addParam<MooseEnum>(
      "direction", direction, "Coordinate direction along which the interface position is reported.");
  params.addParam<MooseEnum>(
      "extremum_type", extremum_type, "Whether to report the maximum or minimum interface position.");
  params.addParam<Real>("threshold", 0.5, "The volume-fraction threshold that defines the interface.");
  params.addRangeCheckedParam<Real>(
      "minimum_alignment",
      0.5,
      "minimum_alignment>=0 & minimum_alignment<=1",
      "Minimum absolute projection of the face normal onto the requested direction.");
  params.addParam<Real>(
      "secondary_min",
      -std::numeric_limits<Real>::max(),
      "Lower bound on the first orthogonal coordinate of sampled face centroids.");
  params.addParam<Real>(
      "secondary_max",
      std::numeric_limits<Real>::max(),
      "Upper bound on the first orthogonal coordinate of sampled face centroids.");
  params.addParam<Real>(
      "tertiary_min",
      -std::numeric_limits<Real>::max(),
      "Lower bound on the second orthogonal coordinate of sampled face centroids.");
  params.addParam<Real>(
      "tertiary_max",
      std::numeric_limits<Real>::max(),
      "Upper bound on the second orthogonal coordinate of sampled face centroids.");
  params.addParam<Real>("value_if_no_interface",
                        0.0,
                        "Value returned if no threshold crossing or fallback liquid cell is found.");
  params.addClassDescription("Computes an extremal subcell interface position from neighboring FV "
                             "cell-centered volume fractions.");
  return params;
}

SubcellInterfacialPosition::SubcellInterfacialPosition(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _volume_fraction(getFunctor<Real>("volume_fraction")),
    _direction(static_cast<unsigned int>(getParam<MooseEnum>("direction"))),
    _search_max(getParam<MooseEnum>("extremum_type") == "max"),
    _threshold(getParam<Real>("threshold")),
    _minimum_alignment(getParam<Real>("minimum_alignment")),
    _secondary_min(getParam<Real>("secondary_min")),
    _secondary_max(getParam<Real>("secondary_max")),
    _tertiary_min(getParam<Real>("tertiary_min")),
    _tertiary_max(getParam<Real>("tertiary_max")),
    _value_if_no_interface(getParam<Real>("value_if_no_interface")),
    _value(_value_if_no_interface),
    _found_candidate(false)
{
  if (_direction >= _mesh.dimension())
    paramError("direction",
               "Requested direction is incompatible with mesh dimension ",
               _mesh.dimension(),
               ".");
}

void
SubcellInterfacialPosition::initialize()
{
  _value = _search_max ? -std::numeric_limits<Real>::max() : std::numeric_limits<Real>::max();
  _found_candidate = false;
}

bool
SubcellInterfacialPosition::withinFilterWindow(const Point & point) const
{
  static constexpr unsigned int filter_axes[3][2] = {{1, 2}, {0, 2}, {0, 1}};
  const auto axis_a = filter_axes[_direction][0];
  const auto axis_b = filter_axes[_direction][1];

  const bool matches_secondary = point(axis_a) >= _secondary_min && point(axis_a) <= _secondary_max;
  const bool matches_tertiary =
      (_mesh.dimension() < 3 && axis_b == 2) ||
      (point(axis_b) >= _tertiary_min && point(axis_b) <= _tertiary_max);

  return matches_secondary && matches_tertiary;
}

void
SubcellInterfacialPosition::updateCandidate(const Real value)
{
  if (!_found_candidate)
  {
    _value = value;
    _found_candidate = true;
    return;
  }

  if (_search_max)
    _value = std::max(_value, value);
  else
    _value = std::min(_value, value);
}

void
SubcellInterfacialPosition::execute()
{
  const auto time_arg = determineState();

  for (const auto * const fi : _mesh.faceInfo())
  {
    if (!fi || !fi->elemPtr() || !fi->neighborPtr())
      continue;

    if (!withinFilterWindow(fi->faceCentroid()))
      continue;

    const auto normal = fi->normal();
    const Real alignment = std::abs(normal(_direction)) / std::max(normal.norm(), libMesh::TOLERANCE);
    if (alignment < _minimum_alignment)
      continue;

    const Point elem_centroid = fi->elemCentroid();
    const Point neighbor_centroid = fi->neighborCentroid();

    const Real elem_alpha =
        clampVolumeFraction(MetaPhysicL::raw_value(_volume_fraction(makeElemArg(fi->elemPtr()), time_arg)));
    const Real neighbor_alpha = clampVolumeFraction(
        MetaPhysicL::raw_value(_volume_fraction(makeElemArg(fi->neighborPtr()), time_arg)));

    const Real elem_position = elem_centroid(_direction);
    const Real neighbor_position = neighbor_centroid(_direction);

    if (elem_alpha >= _threshold)
      updateCandidate(elem_position);
    if (neighbor_alpha >= _threshold)
      updateCandidate(neighbor_position);

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

    const Real crossing_position =
        elem_position + interpolation * (neighbor_position - elem_position);
    updateCandidate(crossing_position);
  }
}

void
SubcellInterfacialPosition::finalize()
{
  if (_search_max)
    gatherMax(_value);
  else
    gatherMin(_value);

  bool found_candidate = _found_candidate;
  _communicator.max(found_candidate);
  _found_candidate = found_candidate;

  if (!_found_candidate)
    _value = _value_if_no_interface;
}

Real
SubcellInterfacialPosition::getValue() const
{
  return _value;
}
