//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HexagonalLatticeUtils.h"
#include "MooseUtils.h"
#include "GeometryUtils.h"

const Real HexagonalLatticeUtils::COS60 = 0.5;
const Real HexagonalLatticeUtils::SIN60 = std::sqrt(3.0) / 2.0;
const unsigned int HexagonalLatticeUtils::NUM_SIDES = 6;

HexagonalLatticeUtils::HexagonalLatticeUtils(const Real bundle_inner_flat_to_flat,
                                             const Real pin_pitch,
                                             const Real pin_diameter,
                                             const Real wire_diameter,
                                             const Real wire_pitch,
                                             const unsigned int n_rings,
                                             const unsigned int axis)
  : _bundle_pitch(bundle_inner_flat_to_flat),
    _pin_pitch(pin_pitch),
    _pin_diameter(pin_diameter),
    _wire_diameter(wire_diameter),
    _wire_pitch(wire_pitch),
    _n_rings(n_rings),
    _axis(axis),
    _bundle_side_length(hexagonSide(_bundle_pitch)),
    _pin_area(M_PI * _pin_diameter * _pin_diameter / 4.0),
    _pin_circumference(M_PI * _pin_diameter),
    _wire_area(M_PI * _wire_diameter * _wire_diameter / 4.0),
    _wire_circumference(M_PI * _wire_diameter),
    _pin_surface_area_per_pitch(M_PI * _pin_diameter * _wire_pitch),
    _pin_volume_per_pitch(_pin_area * _wire_pitch),
    _P_over_D(_pin_pitch / _pin_diameter),
    _L_over_D(_wire_pitch / _pin_diameter)
{
  auto idx = geom_utils::projectedIndices(_axis);
  _ix = idx.first;
  _iy = idx.second;

  // object is not tested and probably won't work if axis != 2
  if (_axis != 2)
    mooseError("The HexagonalLatticeUtils is currently limited to 'axis = 2'!");

  if (_pin_pitch < _pin_diameter)
    mooseError("Pin pitch of " + std::to_string(_pin_pitch) + " cannot fit pins of diameter " +
               std::to_string(_pin_diameter) + "!");

  if (_pin_pitch - _pin_diameter < _wire_diameter)
    mooseError("Wire diameter of " + std::to_string(_wire_diameter) +
               " cannot fit between pin-pin space of " +
               std::to_string(_pin_pitch - _pin_diameter) + "!");

  _unit_translation_x = {0.0, -SIN60, -SIN60, 0.0, SIN60, SIN60};
  _unit_translation_y = {1.0, COS60, -COS60, -1.0, -COS60, COS60};

  // compute number of each pin and channel and channel type (interior, edge channel)
  computePinAndChannelTypes();

  _corner_edge_length = (_bundle_side_length - _n_edge_channels / NUM_SIDES * _pin_pitch) / 2.0;

  computePinBundleSpacing();
  computeWireVolumeAndAreaPerPitch();
  computeFlowVolumes();
  computeWettedAreas();
  computeHydraulicDiameters();
  computePinAndDuctCoordinates();
  computeChannelPinIndices();
  computeGapIndices();

  if (_pin_bundle_spacing < _wire_diameter)
    mooseError("Specified bundle pitch " + std::to_string(_bundle_pitch) +
               " is too small to fit the given pins and wire wrap!");
}

unsigned int
HexagonalLatticeUtils::pins(const unsigned int n) const
{
  if (n <= 0)
    return 0;
  else if (n == 1)
    return 1;
  else
    return NUM_SIDES * (n - 1);
}

unsigned int
HexagonalLatticeUtils::totalPins(const unsigned int n) const
{
  unsigned int total = 0;
  for (unsigned int i = 1; i <= n; ++i)
    total += pins(i);

  return total;
}

unsigned int
HexagonalLatticeUtils::rings(const unsigned int n) const
{
  auto remaining = n;
  unsigned int i = 0;

  while (remaining > 0)
  {
    i += 1;
    remaining -= pins(i);
  }

  if (n != totalPins(i))
    mooseError("Number of pins " + std::to_string(n) +
               " not evenly divisible in a hexagonal lattice!");

  return i;
}

Real
HexagonalLatticeUtils::hexagonSide(const Real pitch) const
{
  return pitch / std::sqrt(3.0);
}

Real
HexagonalLatticeUtils::hexagonArea(const Real pitch) const
{
  Real side = hexagonSide(pitch);
  return 3 * SIN60 * side * side;
}

Real
HexagonalLatticeUtils::hexagonVolume(const Real pitch) const
{
  return hexagonArea(pitch) * _wire_pitch;
}

Real
HexagonalLatticeUtils::hexagonPitch(const Real volume) const
{
  Real area = volume / _wire_pitch;
  return std::sqrt(area / SIN60);
}

Real
HexagonalLatticeUtils::triangleArea(const Real side) const
{
  return 0.5 * triangleHeight(side) * side;
}

Real
HexagonalLatticeUtils::triangleHeight(const Real side) const
{
  return SIN60 * side;
}

Real
HexagonalLatticeUtils::triangleSide(const Real height) const
{
  return height / SIN60;
}

Real
HexagonalLatticeUtils::triangleVolume(const Real side) const
{
  return triangleArea(side) * _wire_pitch;
}

Real
HexagonalLatticeUtils::pinRadius() const
{
  return _pin_diameter / 2.0;
}

channel_type::ChannelTypeEnum
HexagonalLatticeUtils::channelType(const Point & p) const
{
  const Real distance_to_wall = minDuctWallDistance(p);

  if (distance_to_wall > _pin_bundle_spacing + pinRadius())
    return channel_type::interior;

  // the point is in a corner channel if the right triangle formed by the point,
  // the corner, and the duct wall has a wall length less than the wall length
  // of a corner channel
  const Real distance_to_corner = minDuctCornerDistance(p);
  Real l = std::sqrt(distance_to_corner * distance_to_corner - distance_to_wall * distance_to_wall);

  if (l < _corner_edge_length)
    return channel_type::corner;
  else
    return channel_type::edge;
}

Real
HexagonalLatticeUtils::channelSpecificSurfaceArea(
    const channel_type::ChannelTypeEnum & channel) const
{
  switch (channel)
  {
    case channel_type::interior:
      return _interior_wetted_area / _interior_volume;
    case channel_type::edge:
      return _edge_wetted_area / _edge_volume;
    case channel_type::corner:
      return _corner_wetted_area / _corner_volume;
    default:
      mooseError("Unhandled ChannelTypeEnum in HexagonalLattice!");
  }
}

Real
HexagonalLatticeUtils::channelHydraulicDiameter(const channel_type::ChannelTypeEnum & channel) const
{
  switch (channel)
  {
    case channel_type::interior:
      return _interior_Dh;
    case channel_type::edge:
      return _edge_Dh;
    case channel_type::corner:
      return _corner_Dh;
    default:
      mooseError("Unhandled ChannelTypeEnum in HexagonalLattice!");
  }
}

Real
HexagonalLatticeUtils::minDuctWallDistance(const Point & p) const
{
  Real distance = std::numeric_limits<Real>::max();
  for (const auto i : make_range(NUM_SIDES))
  {
    Real a = _duct_coeffs[i][0];
    Real b = _duct_coeffs[i][1];
    Real c = _duct_coeffs[i][2];

    Real d = std::abs(a * p(_ix) + b * p(_iy) + c) / std::sqrt(a * a + b * b);
    distance = std::min(d, distance);
  }

  return distance;
}

Real
HexagonalLatticeUtils::minDuctCornerDistance(const Point & p) const
{
  return geom_utils::minDistanceToPoints(p, _duct_corners, _axis);
}

void
HexagonalLatticeUtils::computePinAndDuctCoordinates()
{
  Real corner_shiftx[] = {COS60, -COS60, -1, -COS60, COS60, 1};
  Real corner_shifty[] = {SIN60, SIN60, 0, -SIN60, -SIN60, 0};

  Real edge_shiftx[] = {-1, -COS60, COS60, 1, COS60, -COS60};
  Real edge_shifty[] = {0, -SIN60, -SIN60, 0, SIN60, SIN60};

  // compute coordinates of the pin centers relative to the bundle's center
  Point center(0, 0, 0);
  _pin_centers.push_back(center);

  for (unsigned int i = 2; i <= _n_rings; ++i)
  {
    auto n_total_in_ring = pins(i);
    auto increment = i - 1;

    unsigned int d = 0;

    for (const auto j : make_range(n_total_in_ring))
    {
      unsigned int side = j / increment;

      if (d == increment)
        d = 0;

      Point center = geom_utils::projectPoint(corner_shiftx[side] * _pin_pitch * (i - 1),
                                              corner_shifty[side] * _pin_pitch * (i - 1),
                                              _axis);

      // additional shift for the edge sides
      if (d != 0)
      {
        center(_ix) += edge_shiftx[side] * _pin_pitch * d;
        center(_iy) += edge_shifty[side] * _pin_pitch * d;
      }

      _pin_centers.push_back(center);

      d += 1;
    }
  }

  // compute corners of the hexagonal prisms that enclose each pin
  _pin_centered_corner_coordinates.resize(_n_pins);
  auto side = hexagonSide(_pin_pitch);
  for (const auto pin : make_range(_n_pins))
  {
    for (const auto i : make_range(NUM_SIDES))
    {
      Point translation = geom_utils::projectPoint(
          _unit_translation_x[i] * side, _unit_translation_y[i] * side, _axis);

      _pin_centered_corner_coordinates[pin].push_back(translation + _pin_centers[pin]);
    }
  }

  // compute coordinates of duct corners relative to the bundle's center
  Real l = _bundle_side_length;
  for (const auto i : make_range(NUM_SIDES))
  {
    Point corner = geom_utils::projectPoint(corner_shiftx[i] * l, corner_shifty[i] * l, _axis);
    _duct_corners.push_back(corner);
  }

  // compute the equations (a*x + b*y + c) defining each duct wall
  for (const auto i : make_range(NUM_SIDES))
  {
    auto c = i;
    unsigned int n = i == 5 ? 0 : i + 1;
    Real slope = (_duct_corners[n](_iy) - _duct_corners[c](_iy)) /
                 (_duct_corners[n](_ix) - _duct_corners[c](_ix));
    std::vector<Real> coeffs = {-slope, 1.0, slope * _duct_corners[c](_ix) - _duct_corners[c](_iy)};
    _duct_coeffs.push_back(coeffs);
  }
}

void
HexagonalLatticeUtils::computeWettedAreas()
{
  Real rod_area_per_pitch = _pin_surface_area_per_pitch + _wire_surface_area_per_pitch;

  _interior_wetted_area = 0.5 * rod_area_per_pitch;
  _edge_wetted_area = 0.5 * rod_area_per_pitch + _pin_pitch * _wire_pitch;

  Real h = (_bundle_side_length - _n_edge_channels / NUM_SIDES * _pin_pitch) / 2.0;
  _corner_wetted_area = rod_area_per_pitch / 6.0 + 2 * h * _wire_pitch;
  _wetted_area = _n_pins * rod_area_per_pitch + 6.0 * _bundle_side_length * _wire_pitch;
}

void
HexagonalLatticeUtils::computeHydraulicDiameters()
{
  // Because the hydraulic diameter should be different for a bundle with an
  // infinite wire pitch vs. a finite wire pitch (because finite = more wire spooled in),
  // we compute hydraulic diameters based on volumes and areas.
  _interior_Dh = 4 * _interior_flow_volume / _interior_wetted_area;
  _edge_Dh = 4 * _edge_flow_volume / _edge_wetted_area;
  _corner_Dh = 4 * _corner_flow_volume / _corner_wetted_area;
  _Dh = 4 * _flow_volume / _wetted_area;
}

void
HexagonalLatticeUtils::computeFlowVolumes()
{
  // It is assumed that in the interior and edge channels, that a wire is present half
  // of the time (because those channels contain half a pin), while in the corner channels
  // that a wire is present one sixth of the time (because those channels contain one sixth
  // of a pin).
  Real rod_volume_per_pitch = _pin_volume_per_pitch + _wire_volume_per_pitch;

  Real l = _pin_bundle_spacing + pinRadius();

  _interior_volume = triangleVolume(_pin_pitch);
  _edge_volume = _pin_pitch * l * _wire_pitch;
  _corner_volume = l * _corner_edge_length * _wire_pitch;

  _interior_flow_volume = _interior_volume - 0.5 * rod_volume_per_pitch;
  _edge_flow_volume = _edge_volume - 0.5 * rod_volume_per_pitch;
  _corner_flow_volume = _corner_volume - rod_volume_per_pitch / 6.0;

  _flow_volume = hexagonVolume(_bundle_pitch) - _n_pins * rod_volume_per_pitch;
}

unsigned int
HexagonalLatticeUtils::interiorChannels(const unsigned int ring)
{
  return (ring * 2 - 1) * NUM_SIDES;
}

void
HexagonalLatticeUtils::computePinAndChannelTypes()
{
  _n_pins = 0;
  for (unsigned int i = _n_rings; i > 0; --i)
    _n_pins += pins(i);

  // the one-ring case
  _n_interior_pins = _n_pins;
  _n_edge_pins = 0;
  _n_corner_pins = 0;

  _n_corner_channels = NUM_SIDES;
  _n_edge_channels = 0;
  _n_interior_channels = 0;

  if (_n_rings > 1)
  {
    _n_corner_pins = NUM_SIDES;
    _n_edge_pins = (_n_rings - 2) * NUM_SIDES;
    _n_interior_pins -= _n_corner_pins + _n_edge_pins;

    _n_edge_channels = _n_edge_pins + NUM_SIDES;

    for (unsigned int i = 1; i < _n_rings; ++i)
      _n_interior_channels += interiorChannels(i);
  }

  _n_channels = _n_interior_channels + _n_edge_channels + _n_corner_channels;
}

void
HexagonalLatticeUtils::computePinBundleSpacing()
{
  // hexagonal flat-to-flat that *just* encloses the pins (NOT including the wire)
  Real pin_h = 2.0 * (_n_rings - 1) * triangleHeight(_pin_pitch) + _pin_diameter;

  _pin_bundle_spacing = (_bundle_pitch - pin_h) / 2.0;
}

void
HexagonalLatticeUtils::computeWireVolumeAndAreaPerPitch()
{
  unsigned int n_wire_coils = 1;
  Real wire_length_per_coil =
      std::sqrt(_wire_pitch * _wire_pitch + std::pow(M_PI * (_pin_diameter + _wire_diameter), 2.0));
  Real wire_length = wire_length_per_coil * n_wire_coils;
  _wire_volume_per_pitch = _wire_area * wire_length;
  _wire_surface_area_per_pitch = _wire_circumference * wire_length;
}

void
HexagonalLatticeUtils::computeChannelPinIndices()
{
  _interior_channel_pin_indices.resize(_n_interior_channels);
  _edge_channel_pin_indices.resize(_n_edge_channels);
  _corner_channel_pin_indices.resize(_n_corner_channels);

  // 3 pins per interior channel
  for (auto & i : _interior_channel_pin_indices)
    i.resize(3);

  // 2 pins per edge channel
  for (auto & i : _edge_channel_pin_indices)
    i.resize(2);

  // 1 pin per corner channel
  for (auto & i : _corner_channel_pin_indices)
    i.resize(1);

  // for each ring of pins, and for each sector, get the "first" pin index in that ring
  std::vector<std::vector<unsigned int>> starts;
  starts.resize(_n_rings);
  for (const auto i : make_range(_n_rings))
  {
    starts[i].resize(NUM_SIDES);
    starts[i][0] = (i == 0) ? 0 : totalPins(i);

    for (unsigned int j = 1; j < NUM_SIDES; ++j)
      starts[i][j] = starts[i][j - 1] + i;
  }

  unsigned int c = 0;
  for (const auto i : make_range(_n_rings - 1))
  {
    auto channels = interiorChannels(i + 1);
    unsigned int channels_per_sector = channels / NUM_SIDES;

    for (const auto j : make_range(NUM_SIDES))
    {
      auto prev_inner = starts[i][j];
      auto prev_outer = starts[i + 1][j];

      for (const auto k : make_range(channels_per_sector))
      {
        bool downward = k % 2 == 0;

        if (downward)
        {
          _interior_channel_pin_indices[c][0] = prev_inner;
          _interior_channel_pin_indices[c][1] = prev_outer;
          _interior_channel_pin_indices[c][2] = ++prev_outer;
        }
        else
        {
          _interior_channel_pin_indices[c][0] = prev_outer;
          _interior_channel_pin_indices[c][1] = ++prev_inner;
          _interior_channel_pin_indices[c][2] = prev_inner - 1;
        }

        if (j == 5) // last sector
        {
          if (k == channels_per_sector - 1)
          {
            _interior_channel_pin_indices[c][2] = starts[i + 1][0];
            _interior_channel_pin_indices[c][0] = starts[i][0];
          }

          if (k == channels_per_sector - 2 && i > 0)
            _interior_channel_pin_indices[c][1] = starts[i][0];
        }

        c++;
      }
    }
  }

  for (const auto i : make_range(_n_edge_channels))
  {
    _edge_channel_pin_indices[i][0] = starts[_n_rings - 1][0] + i;
    _edge_channel_pin_indices[i][1] = _edge_channel_pin_indices[i][0] + 1;

    if (i == _n_edge_channels - 1)
      _edge_channel_pin_indices[i][1] = _edge_channel_pin_indices[0][0];
  }

  for (const auto i : make_range(_n_corner_channels))
    _corner_channel_pin_indices[i][0] =
        totalPins(_n_rings - 1) + i * (_n_edge_channels / NUM_SIDES);
}

const std::vector<Point>
HexagonalLatticeUtils::interiorChannelCornerCoordinates(
    const unsigned int interior_channel_id) const
{
  std::vector<Point> corners;
  auto pin_indices = _interior_channel_pin_indices[interior_channel_id];
  for (const auto & pin : pin_indices)
    corners.push_back(_pin_centers[pin]);

  return corners;
}

const std::vector<Point>
HexagonalLatticeUtils::edgeChannelCornerCoordinates(const unsigned int edge_channel_id) const
{
  std::vector<Point> corners;

  auto pin_indices = _edge_channel_pin_indices[edge_channel_id];

  const Point & pin1 = _pin_centers[pin_indices[0]];
  const Point & pin2 = _pin_centers[pin_indices[1]];

  Real d = pinBundleSpacing() + pinRadius();

  corners.push_back(pin1);
  corners.push_back(pin2);

  unsigned int sector = edge_channel_id / (_n_rings - 1);
  corners.push_back(pin2 +
                    Point(d * _unit_translation_x[sector], d * _unit_translation_y[sector], 0.0));
  corners.push_back(pin1 +
                    Point(d * _unit_translation_x[sector], d * _unit_translation_y[sector], 0.0));

  return corners;
}

const std::vector<Point>
HexagonalLatticeUtils::cornerChannelCornerCoordinates(const unsigned int corner_channel_id) const
{
  std::vector<Point> corners;

  auto pin_indices = _corner_channel_pin_indices[corner_channel_id];
  const Point & pin = _pin_centers[pin_indices[0]];
  corners.push_back(pin);

  Real d = pinBundleSpacing() + pinRadius();

  unsigned int side1 = corner_channel_id == 0 ? NUM_SIDES - 1 : corner_channel_id - 1;
  unsigned int side2 = corner_channel_id;

  corners.push_back(pin +
                    Point(d * _unit_translation_x[side1], d * _unit_translation_y[side1], 0.0));
  corners.push_back(_duct_corners[corner_channel_id]);
  corners.push_back(pin +
                    Point(d * _unit_translation_x[side2], d * _unit_translation_y[side2], 0.0));

  return corners;
}

Point
HexagonalLatticeUtils::channelCentroid(const std::vector<Point> & pins) const
{
  int n_pins = pins.size();
  Point centroid(0.0, 0.0, 0.0);
  for (const auto & p : pins)
    centroid += p / n_pins;

  return centroid;
}

unsigned int
HexagonalLatticeUtils::pinIndex(const Point & point) const
{
  auto side = hexagonSide(_pin_pitch);

  for (const auto i : make_range(_n_pins))
  {
    const auto & center = _pin_centers[i];
    Real dx = center(_ix) - point(_ix);
    Real dy = center(_iy) - point(_iy);
    Real distance_from_pin = std::sqrt(dx * dx + dy * dy);

    // if we're outside the circumference of the hexagon, we're certain not
    // within the hexagon for this pin
    if (distance_from_pin > side)
      continue;

    auto corners = _pin_centered_corner_coordinates[i];
    if (geom_utils::pointInPolygon(point, corners, _axis))
      return i;
  }

  return _n_pins;
}

unsigned int
HexagonalLatticeUtils::closestPinIndex(const Point & point) const
{
  // If within the lattice, you must consider all pins. If outside, outer ring suffices
  unsigned int start_index = 0;
  if (!insideLattice(point))
    start_index = firstPinInRing(_n_rings);

  // Classic minimum search. If more performance is required for a large lattice we may consider
  // using a KD-Tree instead or checking which ring the point is part of before examining each pin
  auto min_distance = std::numeric_limits<Real>::max();
  unsigned int index_min = 0;
  for (unsigned int i = start_index; i < _n_pins; ++i)
  {
    const auto & center = _pin_centers[i];
    Real dx = center(_ix) - point(_ix);
    Real dy = center(_iy) - point(_iy);
    Real distance_from_pin = std::sqrt(dx * dx + dy * dy);

    if (distance_from_pin < min_distance)
    {
      min_distance = distance_from_pin;
      index_min = i;
    }
  }

  return index_min;
}

unsigned int
HexagonalLatticeUtils::channelIndex(const Point & point) const
{
  auto channel = channelType(point);

  switch (channel)
  {
    case channel_type::interior:
    {
      for (const auto i : make_range(_n_interior_channels))
      {
        auto corners = interiorChannelCornerCoordinates(i);
        if (geom_utils::pointInPolygon(point, corners, _axis))
          return i;
      }
      break;
    }
    case channel_type::edge:
    {
      for (const auto i : make_range(_n_edge_channels))
      {
        auto corners = edgeChannelCornerCoordinates(i);
        if (geom_utils::pointInPolygon(point, corners, _axis))
          return i + _n_interior_channels;
      }
      break;
    }
    case channel_type::corner:
    {
      for (const auto i : make_range(_n_corner_channels))
      {
        auto corners = cornerChannelCornerCoordinates(i);
        if (geom_utils::pointInPolygon(point, corners, _axis))
          return i + _n_interior_channels + _n_edge_channels;
      }
      break;
    }
    default:
      mooseError("Unhandled ChannelTypeEnum!");
  }

  mooseError(
      "Point (" + std::to_string(point(0)) + ", " + std::to_string(point(1)) + ", " +
      std::to_string(point(2)) +
      ") is not in any channel! This can sometimes happen "
      "due to:\n\n a) Points in the mesh actually being outside the domain specified with the "
      "HexagonalLatticeUtils.\n b) Small floating point errors - we recommend using a CONSTANT "
      "MONOMIAL variable with all related objects.\nYou can also try slightly decreasing the pin "
      "diameter and/or "
      "increasing the bundle pitch.");
}

bool
HexagonalLatticeUtils::insideLattice(const Point & point) const
{
  std::vector<Point> lattice_corners(6);
  auto side = hexagonSide(_bundle_pitch);
  for (const auto i : make_range(NUM_SIDES))
  {
    Point translation = geom_utils::projectPoint(
        _unit_translation_x[i] * side, _unit_translation_y[i] * side, _axis);
    lattice_corners[i] = translation;
  }
  return geom_utils::pointInPolygon(point, lattice_corners, _axis);
}

void
HexagonalLatticeUtils::get2DInputPatternIndex(const unsigned int pin_index,
                                              unsigned int & row_index,
                                              unsigned int & within_row_index) const
{

  // First compute the ring on which the pin is
  const auto ring_index = ringIndex(pin_index);
  row_index = _n_rings - ring_index;
  within_row_index = _n_rings - 1;

  const auto n_pin_sectors = pins(ring_index) / 6;
  // Loop around the center until we reach the pin_index
  // We start on the diagonal at the top

  for (const auto local_i : make_range(pin_index - firstPinInRing(ring_index)))
  {
    if (local_i < n_pin_sectors)
      within_row_index--;
    else if (local_i < 3 * n_pin_sectors)
      row_index++;
    else if (local_i < 4 * n_pin_sectors)
      within_row_index++;
    else if (local_i < 5 * n_pin_sectors)
    {
      row_index--;
      within_row_index++;
    }
    else
    {
      row_index--;
      within_row_index--;
    }
  }

  // Underflow (row_index is invalid_uint) will also fail this assert
  mooseAssert(row_index < 2 * _n_rings - 1, "Inverse indexing failed");
  mooseAssert(within_row_index < 2 * _n_rings - 1, "Inverse indexing failed");
}

void
HexagonalLatticeUtils::computeGapIndices()
{
  std::set<std::pair<int, int>> indices;
  for (const auto & pins : _interior_channel_pin_indices)
  {
    std::pair<int, int> gap0 = {std::min(pins[0], pins[1]), std::max(pins[0], pins[1])};
    std::pair<int, int> gap1 = {std::min(pins[0], pins[2]), std::max(pins[0], pins[2])};
    std::pair<int, int> gap2 = {std::min(pins[2], pins[1]), std::max(pins[2], pins[1])};

    indices.insert(gap0);
    indices.insert(gap1);
    indices.insert(gap2);
  }

  for (const auto & it : indices)
    _gap_indices.push_back({it.first, it.second});

  _n_interior_gaps = _gap_indices.size();

  // add the gaps along the peripheral channels; -1 indicates side 1, -2 indicates side 2,
  // and so on
  int n_edge_gaps = _n_rings;
  int pin = totalPins(_n_rings - 1);
  for (const auto i : make_range(NUM_SIDES))
  {
    for (const auto j : make_range(_n_rings))
      _gap_indices.push_back({pin + j, -(i + 1)});

    pin += n_edge_gaps - 1;
  }

  // fix the last gap to use the first pin
  _gap_indices.back() = {totalPins(_n_rings - 1), -int(NUM_SIDES)};
  _n_gaps = _gap_indices.size();

  // for each channel, determine which gaps are on that channel to find the local-to-global
  // indexing
  for (const auto & pins : _interior_channel_pin_indices)
  {
    std::pair<int, int> gap0 = {std::min(pins[0], pins[1]), std::max(pins[0], pins[1])};
    std::pair<int, int> gap1 = {std::min(pins[1], pins[2]), std::max(pins[1], pins[2])};
    std::pair<int, int> gap2 = {std::min(pins[2], pins[0]), std::max(pins[2], pins[0])};

    int loc_gap0 = globalGapIndex(gap0);
    int loc_gap1 = globalGapIndex(gap1);
    int loc_gap2 = globalGapIndex(gap2);
    _local_to_global_gaps.push_back({loc_gap0, loc_gap1, loc_gap2});
  }

  int gap = _gap_indices.size() - _n_rings * NUM_SIDES;
  for (const auto i : make_range(_n_edge_channels))
  {
    const auto & pins = _edge_channel_pin_indices[i];
    std::pair<int, int> gap0 = {std::min(pins[0], pins[1]), std::max(pins[0], pins[1])};
    int loc_gap0 = globalGapIndex(gap0);
    _local_to_global_gaps.push_back({loc_gap0, gap + 1, gap});

    if ((i + 1) % (_n_rings - 1) == 0)
      gap += 2;
    else
      gap += 1;
  }

  int n_interior_gaps = _gap_indices.size() - _n_rings * NUM_SIDES - 1;
  n_edge_gaps = _n_rings * NUM_SIDES;
  _local_to_global_gaps.push_back({n_interior_gaps + n_edge_gaps, n_interior_gaps + 1});
  gap = n_interior_gaps + _n_rings;
  for (unsigned int i = 1; i < _n_corner_channels; ++i)
  {
    _local_to_global_gaps.push_back({gap, gap + 1});
    gap += _n_rings;
  }

  _gap_points.resize(_n_gaps);

  // For each gap, get two points on the gap
  for (const auto i : make_range(_n_interior_gaps))
  {
    const auto & pins = _gap_indices[i];
    Point pt1(_pin_centers[pins.first]);
    Point pt2(_pin_centers[pins.second]);
    _gap_centers.push_back(0.5 * (pt2 + pt1));

    _gap_points[i] = {pt1, pt2};

    // for the last gap in the ring, we need to swap the ordering of pins
    if (lastGapInRing(i))
    {
      Point tmp = pt1;
      pt1 = pt2;
      pt2 = tmp;
    }

    _gap_unit_normals.push_back(geom_utils::projectedUnitNormal(pt1, pt2, _axis));
  }

  Real d = _pin_bundle_spacing + pinRadius();
  for (unsigned int i = _n_interior_gaps; i < _n_gaps; ++i)
  {
    const auto & pins = _gap_indices[i];
    int side = std::abs(pins.second) - 1;

    const auto & pt1 = _pin_centers[pins.first];
    const Point pt2 =
        pt1 + Point(d * _unit_translation_x[side], d * _unit_translation_y[side], 0.0);
    _gap_centers.push_back(0.5 * (pt2 + pt1));

    _gap_points[i] = {pt1, pt2};

    _gap_unit_normals.push_back(geom_utils::projectedUnitNormal(pt1, pt2, _axis));
  }
}

unsigned int
HexagonalLatticeUtils::globalGapIndex(const std::pair<int, int> & local_gap) const
{
  for (const auto i : index_range(_gap_indices))
  {
    const auto gap = _gap_indices[i];
    if (gap.first == local_gap.first && gap.second == local_gap.second)
      return i;
  }

  mooseError("Failed to find local gap in global gap array!");
}

Real
HexagonalLatticeUtils::distanceFromGap(const Point & pt, const unsigned int gap_index) const
{
  auto p = _gap_points[gap_index];
  return geom_utils::projectedDistanceFromLine(pt, p[0], p[1], _axis);
}

unsigned int
HexagonalLatticeUtils::gapIndex(const Point & point) const
{
  const auto & channel_index = channelIndex(point);
  const auto & gap_indices = _local_to_global_gaps[channel_index];

  Real distance = std::numeric_limits<Real>::max();
  unsigned int index = 0;
  for (const auto i : index_range(gap_indices))
  {
    Real distance_from_gap = distanceFromGap(point, gap_indices[i]);

    if (distance_from_gap < distance)
    {
      distance = distance_from_gap;
      index = gap_indices[i];
    }
  }

  return index;
}

void
HexagonalLatticeUtils::gapIndexAndDistance(const Point & point,
                                           unsigned int & index,
                                           Real & distance) const
{
  const auto & channel_index = channelIndex(point);
  const auto & gap_indices = _local_to_global_gaps[channel_index];

  distance = std::numeric_limits<Real>::max();
  for (const auto i : index_range(gap_indices))
  {
    Real distance_from_gap = distanceFromGap(point, gap_indices[i]);

    if (distance_from_gap < distance)
    {
      distance = distance_from_gap;
      index = gap_indices[i];
    }
  }
}

unsigned int
HexagonalLatticeUtils::firstPinInRing(const unsigned int ring) const
{
  mooseAssert(ring > 0, "Ring indexing starts at 1");
  if (ring == 1)
    return 0;
  else
    return totalPins(ring - 1);
}

unsigned int
HexagonalLatticeUtils::lastPinInRing(const unsigned int ring) const
{
  mooseAssert(ring > 0, "Ring indexing starts at 1");
  if (ring == 1)
    return 0;
  else
    return totalPins(ring) - 1;
}

unsigned int
HexagonalLatticeUtils::ringIndex(const unsigned int pin) const
{
  for (const auto i : make_range(_n_rings))
  {
    if (pin <= lastPinInRing(i + 1))
      return i + 1;
  }
  mooseError("Should not reach here. Pin index: ", pin, " for ", _n_rings, " rings.");
}

bool
HexagonalLatticeUtils::lastGapInRing(const unsigned int gap_index) const
{
  if (gap_index >= _n_interior_gaps)
    return false;

  const auto & pins = _gap_indices[gap_index];

  for (unsigned int i = 2; i <= _n_rings; ++i)
  {
    bool one_is_first_pin = false;
    bool one_is_last_pin = false;

    int first_pin = firstPinInRing(i);
    int last_pin = lastPinInRing(i);

    if (pins.first == first_pin || pins.second == first_pin)
      one_is_first_pin = true;

    if (pins.first == last_pin || pins.second == last_pin)
      one_is_last_pin = true;

    if (one_is_first_pin && one_is_last_pin)
      return true;
  }

  return false;
}
