//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"

#ifndef CARDINAL_ENUM_H
namespace channel_type
{
/// Type of subchannel
enum ChannelTypeEnum
{
  interior,
  edge,
  corner
};
} // namespace channel_type
#endif

/**
 * Class providing various utility functions related to triangular lattices of pins
 * enclosed in a hexagonal duct.
 * Notes:
 * - the lattice is centered around an axis (X, Y, Z) going through the origin
 *   If calling the utility for a lattice that is not centered around the origin
 *   Then the point arguments must first be translated: p -= lattice_center
 * - You should never have to rotate the point to place it in the lattice frame
 *   before calling a routine with a point argument.
 */
class HexagonalLatticeUtils
{
public:
  HexagonalLatticeUtils(const Real bundle_inner_flat_to_flat,
                        const Real pin_pitch,
                        const Real pin_diameter,
                        const Real wire_diameter,
                        const Real wire_pitch,
                        const unsigned int n_rings,
                        const unsigned int axis);

  /**
   * Distance from a point and a gap
   * @param[in] pt point, must already be translated to lattice frame
   * @param[in] gap_index gap index
   * @ return distance from gap
   */
  Real distanceFromGap(const Point & pt, const unsigned int gap_index) const;

  /**
   * Get the index for the gap closest to the point
   * @param[in] point point, must already be translated to lattice frame
   * @return index of closest gap
   */
  unsigned int gapIndex(const Point & point) const;

  /**
   * Get the gap index and distance to that gap for a given point
   * @param[in] point point, must already be translated to lattice frame
   * @param[out] index index of closest gap
   * @param[out] distance distance to closest gap
   */
  void gapIndexAndDistance(const Point & point, unsigned int & index, Real & distance) const;

  /**
   * Get the unit vector translation to move a center point to a duct wall
   * @param[in] side duct side
   * @return x-component translation
   */
  Real sideTranslationX(const unsigned int side) const { return _unit_translation_x[side]; }

  /**
   * Get the unit vector translation to move a center point to a duct wall
   * @param[in] side duct side
   * @return y-component translation
   */
  Real sideTranslationY(const unsigned int side) const { return _unit_translation_y[side]; }

  /**
   * Get the index of the "first" pin in a ring
   * @param[in] ring ring index
   * @return first pin
   */
  unsigned int firstPinInRing(const unsigned int ring) const;

  /**
   * Get the index of the "last" pin in a ring
   * @param[in] ring ring index
   * @return last pin
   */
  unsigned int lastPinInRing(const unsigned int ring) const;

  /**
   * Whether this gap is the "last" in the ring, i.e. connecting the first and last pins of the ring
   * @param[in] gap_index gap index
   * @return whether this gap is the last in the ring
   */
  bool lastGapInRing(const unsigned int gap_index) const;

  /**
   * Get the index of the ring for a certain pin index
   * @param[in] pin pin index
   * @return ring index
   */
  unsigned int ringIndex(const unsigned int pin) const;

  /**
   * Get the unit normals for all of the gaps
   * @return gap unit normals
   */
  const std::vector<Point> & gapUnitNormals() const { return _gap_unit_normals; }

  /**
   * Get the number of pins in a given ring
   * @param[in] n ring number, beginning from 1
   * @return number of pins in the specified ring
   */
  unsigned int pins(const unsigned int n) const;

  /**
   * Get the total number of pins for all rings
   * @param[in] n number of total rings, beginning from 1
   * @return total number of pins
   */
  unsigned int totalPins(const unsigned int n) const;

  /**
   * Get the number of rings for a specified number of pins
   * @param[in] n number of pins
   * @return number of hexagonal rings
   */
  unsigned int rings(const unsigned int n) const;

  /**
   * Get the number of interior channels between ring and ring - 1 (0 indexing)
   * @param[in] ring ring number (relative to 0 indexing)
   * @return number of interior channels
   */
  unsigned int interiorChannels(const unsigned int ring);

  /**
   * Get the number of gaps that touch an interior channel
   * @return number of gaps touching an interior channel
   */
  unsigned int nInteriorGaps() const { return _n_interior_gaps; }

  /**
   * Get the center coordinates of the gaps
   * @return gap center coordinates
   */
  const std::vector<Point> & gapCenters() const { return _gap_centers; }

  /**
   * Get the number of gaps
   * @return number of gaps
   */
  unsigned int nGaps() const { return _n_gaps; }

  /**
   * Get the (pin) pitch-to-diameter ratio
   * @return pin pitch-to-diameter ratio
   */
  Real pitchToDiameter() const { return _P_over_D; }

  /**
   * Get the (wire) axial pitch-to-diameter ratio
   * @return wire axial pitch-to-diameter ratio
   */
  Real heightToDiameter() const { return _L_over_D; }

  /**
   * Get the vertical axis of the bed along which pins are aligned
   * @return vertical axis
   */
  unsigned int axis() const { return _axis; }

  /**
   * Get the pin diameter
   * @return pin diameter
   */
  Real pinDiameter() const { return _pin_diameter; }

  /**
   * Get the wire pitch
   * @return wire pitch
   */
  Real wirePitch() const { return _wire_pitch; }

  /**
   * Get the wire diameter
   * @return wire diameter
   */
  Real wireDiameter() const { return _wire_diameter; }

  /**
   * Get the pin pitch
   * @return pin pitch
   */
  Real pinPitch() const { return _pin_pitch; }

  /**
   * Get the bundle pitch
   * @return bundle pitch
   */
  Real bundlePitch() const { return _bundle_pitch; }

  /**
   * Get the pin volume per pitch
   * @return pin volume per pitch
   */
  Real pinVolumePerPitch() const { return _pin_volume_per_pitch; }

  /**
   * Get the wire volume per pitch
   * @return wire volume per pitch
   */
  Real wireVolumePerPitch() const { return _wire_volume_per_pitch; }

  /**
   * Get the number of interior pins
   * @return number of interior pins
   */
  unsigned int nInteriorPins() const { return _n_interior_pins; }

  /**
   * Get the number of edge pins
   * @return number of edge pins
   */
  unsigned int nEdgePins() const { return _n_edge_pins; }

  /**
   * Get the number of corner pins
   * @return number of corner pins
   */
  unsigned int nCornerPins() const { return _n_corner_pins; }

  /**
   * Get the total number of pins for the lattice
   * @return total number of pins
   */
  unsigned int nPins() const { return _n_pins; }

  /**
   * Get the hydraulic diameter of an interior channel
   * @return hydraulic diameter of an interior channel
   */
  Real interiorHydraulicDiameter() const { return _interior_Dh; }

  /**
   * Get the hydraulic diameter of an edge channel
   * @return hydraulic diameter of an edge channel
   */
  Real edgeHydraulicDiameter() const { return _edge_Dh; }

  /**
   * Get the hydraulic diameter of a corner channel
   * @return hydraulic diameter of a corner channel
   */
  Real cornerHydraulicDiameter() const { return _corner_Dh; }

  /**
   * Get the overall bundle hydraulic diameter
   * @return hydraulic diameter
   */
  Real hydraulicDiameter() const { return _Dh; }

  /**
   * Get the wetted area of an interior channel per wire pitch
   * @return wetted area of interior channel
   */
  Real interiorWettedArea() const { return _interior_wetted_area; }

  /**
   * Get the wetted area of an edge channel per wire pitch
   * @return wetted area of edge channel
   */
  Real edgeWettedArea() const { return _edge_wetted_area; }

  /**
   * Get the wetted area of a corner channel per wire pitch
   * @return wetted area of corner channel
   */
  Real cornerWettedArea() const { return _corner_wetted_area; }

  /**
   * Get the wetted area of entire bundle per wire pitch
   * @return wetted area of bundle
   */
  Real wettedArea() const { return _wetted_area; }

  /**
   * Get the flow volume of an interior channel per wire pitch
   * @return flow volume of interior channel
   */
  Real interiorFlowVolume() const { return _interior_flow_volume; }

  /**
   * Get the flow volume of an edge channel per wire pitch
   * @return flow volume of edge channel
   */
  Real edgeFlowVolume() const { return _edge_flow_volume; }

  /**
   * Get the flow volume of an corner channel per wire pitch
   * @return flow volume of corner channel
   */
  Real cornerFlowVolume() const { return _corner_flow_volume; }

  /**
   * Get the total volume of an interior channel per wire pitch
   * @return total volume of interior channel
   */
  Real interiorVolume() const { return _interior_volume; }

  /**
   * Get the total volume of an edge channel per wire pitch
   * @return total volume of edge channel
   */
  Real edgeVolume() const { return _edge_volume; }

  /**
   * Get the total volume of an corner channel per wire pitch
   * @return total volume of corner channel
   */
  Real cornerVolume() const { return _corner_volume; }

  /**
   * Get the flow volume of the bundle per wire pitch
   * @return flow volume of bundle
   */
  Real flowVolume() const { return _flow_volume; }

  /**
   * Get the number of interior channels
   * @return number of interior channels
   */
  unsigned int nInteriorChannels() const { return _n_interior_channels; }

  /**
   * Get the number of edge channels
   * @return number of edge channels
   */
  unsigned int nEdgeChannels() const { return _n_edge_channels; }

  /**
   * Get the number of corner channels
   * @return number of corner channels
   */
  unsigned int nCornerChannels() const { return _n_corner_channels; }

  /**
   * Get the total number of channels
   * @return number of channels
   */
  unsigned int nChannels() const { return _n_channels; }

  /**
   * Get the distance between the outermost pins and the duct walls
   * @return pin-bundle spacing
   */
  Real pinBundleSpacing() const { return _pin_bundle_spacing; }

  /**
   * Get the center coordinates of the pins
   * @return pin center coordinates
   */
  const std::vector<Point> & pinCenters() const { return _pin_centers; }

  /**
   * Get the corner coordinates of a hexagon surrounding each pin
   * @return pin center coordinates
   */
  const std::vector<std::vector<Point>> & pinCenteredCornerCoordinates() const
  {
    return _pin_centered_corner_coordinates;
  }

  /**
   * Get the pin surface area per wire pitch
   * @return pin surface area per wire pitch
   */
  Real pinSurfaceAreaPerPitch() const { return _pin_surface_area_per_pitch; }

  /**
   * Get half the distance of the duct that a corner channel is in contact with
   * @return half the duct length that a corner channel is in contact with
   */
  Real cornerEdgeLength() const { return _corner_edge_length; }

  /**
   * Get the minimum distance from a point to the duct inner surface
   * @param[in] p point, must already be translated to lattice frame
   * @return distance to duct
   */
  Real minDuctWallDistance(const Point & p) const;

  /**
   * Get the minimum distance from a point to the duct corners
   * @param[in] p point, must already be translated to lattice frame
   * @return distance to duct corner
   */
  Real minDuctCornerDistance(const Point & p) const;

  /**
   * Get the channel type (interior, edge, corner) given a point
   * @param[in] p point, must already be translated to lattice frame
   * @return channel type
   */
  channel_type::ChannelTypeEnum channelType(const Point & p) const;

  /**
   * Get the specific surface area of a channel
   * @param[in] channel channel type
   * @return flow volume per wire pitch
   */
  Real channelSpecificSurfaceArea(const channel_type::ChannelTypeEnum & channel) const;

  /**
   * Get the hydraulic diameter of a channel
   * @param[in] channel channel type
   * @return hydraulic diameter
   */
  Real channelHydraulicDiameter(const channel_type::ChannelTypeEnum & channel) const;

  /**
   * Get the pin outer radius
   * @return pin outer radius
   */
  Real pinRadius() const;

  /**
   * Get the area of a hexagon with given flat-to-flat distance (pitch)
   * @param[in] pitch flat-to-flat distance
   * @return hexagon area
   */
  Real hexagonArea(const Real pitch) const;

  /**
   * Get the side length of a hexagon with given flat-to-flat distance (pitch)
   * @param[in] pitch hexagon flat-to-flat distance, or pitch
   * @return side length of hexagon, to give 1/6 of the hexagon's perimeter
   */
  Real hexagonSide(const Real pitch) const;

  /**
   * Get the volume of a hexagonal prism per wire pitch
   * @param[in] side side length
   * @return hexagonal prism volume
   */
  Real hexagonVolume(const Real side) const;

  /**
   * Get the pitch of a hexagonal prism based on its per-wire-pitch volume
   * @param[in] volume volume
   * @return pitch
   */
  Real hexagonPitch(const Real volume) const;

  /**
   * Get the area of an equilateral triangle with given side length
   * @param[in] side side length
   * @return triangle area
   */
  Real triangleArea(const Real side) const;

  /**
   * Get the height of an equilateral triangle with given side length
   * @param[in] side side length
   * @return triangle height
   */
  Real triangleHeight(const Real side) const;

  /**
   * Get the side of an equilateral triangle with given height
   * @param[in] height triangle height
   * @return triangle side
   */
  Real triangleSide(const Real height) const;

  /**
   * Get the volume of an equilateral triangle prism per wire pitch
   * @param[in] side side length
   * @return triangle prism volume
   */
  Real triangleVolume(const Real side) const;

  /**
   * Get the pin indices forming the corners of all interior channels
   * @return pin indices forming the corners of all interior channels
   */
  const std::vector<std::vector<unsigned int>> & interiorChannelPinIndices() const
  {
    return _interior_channel_pin_indices;
  }

  /**
   * Get the pin indices forming two of the corners of all edge channels
   * @return pin indices forming two of the corners of all edge channels
   */
  const std::vector<std::vector<unsigned int>> & edgeChannelPinIndices() const
  {
    return _edge_channel_pin_indices;
  }

  /**
   * Get the pin indices forming one of the corners of all corner channels
   * @return pin indices forming one of the corners of all corner channels
   */
  const std::vector<std::vector<unsigned int>> & cornerChannelPinIndices() const
  {
    return _corner_channel_pin_indices;
  }

  /**
   * Get the pin and side indices on each gap
   * @return pin and side indices on each gap
   */
  const std::vector<std::pair<int, int>> & gapIndices() const { return _gap_indices; }

  /**
   * For each subchannel, get the indices of the gaps that touch that subchannel
   * @return indices of gaps touch each channel
   */
  const std::vector<std::vector<int>> & localToGlobalGaps() const { return _local_to_global_gaps; }

  /**
   * Get the corner coordinates of an interior channel given an ID
   * (relative to the start of the interior channels)
   * @param[in] interior_channel_id ID of interior channel
   * @return corner coordinates of channel
   */
  const std::vector<Point>
  interiorChannelCornerCoordinates(const unsigned int interior_channel_id) const;

  /**
   * Get the corner coordinates of an edge channel given an ID
   * (relative to the start of the edge channels)
   * @param[in] edge_channel_id ID of edge channel
   * @return corner coordinates of channel
   */
  const std::vector<Point> edgeChannelCornerCoordinates(const unsigned int edge_channel_id) const;

  /**
   * Get the corner coordinates of a corner channel given an ID
   * (relative to the start of the corner channels)
   * @param[in] corner_channel_id ID of corner channel
   * @return corner coordinates of channel
   */
  const std::vector<Point>
  cornerChannelCornerCoordinates(const unsigned int corner_channel_id) const;

  /**
   * Get the centroid of a channel given the corner coordinates
   * @param[in] corners corner coordinates
   * @return channel centroid
   */
  Point channelCentroid(const std::vector<Point> & corners) const;

  /**
   * Get the pin index given a point. The pin index is for the entire hexagon around the pin
   * @param[in] point point, must already be translated to lattice frame
   * @return pin index
   */
  unsigned int pinIndex(const Point & point) const;

  /**
   * Get the closest pin index given a point outside the lattice
   * @param[in] point point, must already be translated to lattice frame
   * @return pin index
   */
  unsigned int closestPinIndex(const Point & point) const;

  /**
   * Get the channel index given a point
   * @param[in] point point, must already be translated to lattice frame
   * @return channel index
   */
  unsigned int channelIndex(const Point & point) const;

  /**
   * Whether the point is inside the lattice
   * @param point point being examined, must already be translated to lattice frame
   */
  bool insideLattice(const Point & point) const;

  /**
   * Conversion from lattice pin indexing to the 2D input file index. The indexing matches like
   * this. Note that you have a 30 degree rotation between the 2D input and the positions output
   *  2 1
   * 3 0 6
   *  4 5
   * @param pin_index index of the pin in the utility ring-wise indexing
   * @param row_index row index (from the top) in the 2D input
   * @param within_row_index index within the row
   */
  void get2DInputPatternIndex(const unsigned int pin_index,
                              unsigned int & row_index,
                              unsigned int & within_row_index) const;

protected:
  /**
   * Get the global gap index from the local gap index
   * @param[in] local_gap local gap for a channel
   * @return global gap index
   */
  unsigned int globalGapIndex(const std::pair<int, int> & local_gap) const;

  /// Bundle pitch (distance across bundle measured flat-to-flat on the inside of the duct)
  const Real _bundle_pitch;

  /// Pin pitch
  const Real _pin_pitch;

  /// Pin diameter
  const Real _pin_diameter;

  /// Wire diameter
  const Real _wire_diameter;

  /// Wire pitch
  const Real _wire_pitch;

  /// Total number of rings of pins
  const unsigned int _n_rings;

  /// Vertical axis of the bundle along which the pins are aligned
  const unsigned int _axis;

  /// Side length of duct
  const Real _bundle_side_length;

  /// Pin cross-sectional area
  const Real _pin_area;

  /// Pin circumference
  const Real _pin_circumference;

  /// Wire cross-sectional area
  const Real _wire_area;

  /// Wire circumference
  const Real _wire_circumference;

  /// Pin surface area per wire pitch
  const Real _pin_surface_area_per_pitch;

  /// Single-pin volume per wire pitch
  const Real _pin_volume_per_pitch;

  /// Pitch-to-diameter ratio
  const Real _P_over_D;

  /// Wire axial lead length to diameter ratio
  const Real _L_over_D;

  /// Wire surface area per wire pitch
  Real _wire_surface_area_per_pitch;

  /// Spacing between the duct inner wall and the pin surface
  Real _pin_bundle_spacing;

  /// Single-wire volume per wire pitch
  Real _wire_volume_per_pitch;

  /// Total number of pins
  unsigned int _n_pins;

  /// Number of interior pins
  unsigned int _n_interior_pins;

  /// Number of edge pins
  unsigned int _n_edge_pins;

  /// Number of corner pins
  unsigned int _n_corner_pins;

  /// Total number of channels
  unsigned int _n_channels;

  /// Total number of interior channels
  unsigned int _n_interior_channels;

  /// Total number of edge channels
  unsigned int _n_edge_channels;

  /// Total number of corner channels
  unsigned int _n_corner_channels;

  /// Half the distance for which a corner channel is in contact with the duct
  Real _corner_edge_length;

  /// Hydraulic diameter of interior channel
  Real _interior_Dh;

  /// Hydraulic diameter of edge channel
  Real _edge_Dh;

  /// Hydraulic diameter of corner channel
  Real _corner_Dh;

  /// Bundle-wide hydraulic diameter
  Real _Dh;

  /// Flow volume of an interior channel per wire pitch
  Real _interior_flow_volume;

  /// Flow volume of an edge channel per wire pitch
  Real _edge_flow_volume;

  /// Flow volume of a corner channel per wire pitch
  Real _corner_flow_volume;

  /// Total volume of an interior channel per wire pitch
  Real _interior_volume;

  /// Total volume of an edge channel per wire pitch
  Real _edge_volume;

  /// Total volume of a corner channel per wire pitch
  Real _corner_volume;

  /// Bundle-wide flow volume per wire pitch
  Real _flow_volume;

  /// Wetted area of an interior channel per wire pitch
  Real _interior_wetted_area;

  /// Wetted area of an edge channel per wire pitch
  Real _edge_wetted_area;

  /// Wetted area of a corner channel per wire pitch
  Real _corner_wetted_area;

  /// Wetted area of entire bundle per wire pitch
  Real _wetted_area;

  /// Pin center coordinates
  std::vector<Point> _pin_centers;

  /// Corner coordinates of a hexagon surrounding each pin
  std::vector<std::vector<Point>> _pin_centered_corner_coordinates;

  /// Six corner coordinates for the ducts
  std::vector<Point> _duct_corners;

  /**
   * Coefficients in the line equations defining each duct wall
   *
   * Coefficients \f$a\f$, \f$b\f$, and \f$c\f$ in equation \f$ax+by+c=0\f$ that
   * defines each wall of the duct.
   */
  std::vector<std::vector<Real>> _duct_coeffs;

  /// Pin indices forming the corner of each interior channel
  std::vector<std::vector<unsigned int>> _interior_channel_pin_indices;

  /// Pin indices forming two of the corners of each edge channel
  std::vector<std::vector<unsigned int>> _edge_channel_pin_indices;

  /// Pin indices forming one of the corners of each corner channel
  std::vector<std::vector<unsigned int>> _corner_channel_pin_indices;

  /// Gap indices, connecting two pins or one pin and a side, ordered by global gap ID
  std::vector<std::pair<int, int>> _gap_indices;

  /// Local-to-global gap indexing, ordered by channel ID
  std::vector<std::vector<int>> _local_to_global_gaps;

  /// Two points on each gap, in order to compute distance-from-gap calculations
  std::vector<std::vector<Point>> _gap_points;

  /// Unit normal vectors for each gap
  std::vector<Point> _gap_unit_normals;

  /// Number of gaps that touch an interior channel
  unsigned int _n_interior_gaps;

  /// Total number of gaps
  unsigned int _n_gaps;

  static const Real SIN60;

  static const Real COS60;

  /// Number of sides in a hexagon
  static const unsigned int NUM_SIDES;

  /// (unitless) x-translations to apply to move from a center point to a side of a hexagon
  std::vector<Real> _unit_translation_x;

  /// (unitless) y-translations to apply to move from a center point to a side of a hexagon
  std::vector<Real> _unit_translation_y;

  /// Center points of all the gaps
  std::vector<Point> _gap_centers;

  /// Index representing "first" coordinate of 2-D plane
  unsigned int _ix;

  /// Index representing "second" coordinate of 2-D plane
  unsigned int _iy;

private:
  /// Determine the global gap indices, sorted first by lower pin ID and next by higher pin ID
  void computeGapIndices();

  /// Compute the number of pins and channels and their types for the lattice
  void computePinAndChannelTypes();

  /// Compute the spacing between the outer pins and the duct inner walls
  void computePinBundleSpacing();

  /// Compute the volume and surface area const occupied by the wire in one with pitch
  void computeWireVolumeAndAreaPerPitch();

  /// Compute the flow volumes for each channel type
  void computeFlowVolumes();

  /// Compute the wetted areas for each channel type
  void computeWettedAreas();

  /// Compute the hydraulic diameters for each channel type
  void computeHydraulicDiameters();

  /**
   * \brief Compute the pin center coordinates and the duct corner coordinates
   *
   * Compute the x, y, z coordinates of the pincells based on a bundle centered on (0, 0, 0)
   * and with pins aligned in the z direction. Also compute the corner coordinates of the ducts
   */
  void computePinAndDuctCoordinates();

  /// Get the pin indices that form the corners of each channel type
  void computeChannelPinIndices();
};
