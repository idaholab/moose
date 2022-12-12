//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "PolygonMeshGeneratorBase.h"
#include "MooseEnum.h"

/**
 * This PolygonMeshTrimmerBase is the base class for CartesianMeshTrimmer and HexagonMeshTrimmer,
 * which take in a cartesian/hexagonal assembly or core mesh and perform peripheral and/or center
 * trimming on it.
 */
class PolygonMeshTrimmerBase : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PolygonMeshTrimmerBase(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh to be modified
  const MeshGeneratorName _input_name;
  /// Index of the peripheral regions to be trimmed (see moosedocs for indexing scheme)
  std::vector<unsigned short> _trim_peripheral_region;
  /// Name of the section formed by peripheral trimming
  const BoundaryName _peripheral_trimming_section_boundary;
  /// Index of the sector to start trimming from (counter-clockwise direction)
  short _trimming_start_sector;
  /// Index of the sector to end trimming at (counter-clockwise direction)
  short _trimming_end_sector;
  /// Number of remaining sectors
  unsigned short _center_trim_sector_number;
  /// Name of the section formed by center trimming
  const BoundaryName _center_trimming_section_boundary;
  /// Name/ID of external boundary
  const BoundaryName _external_boundary_name;
  /// SubdomainName suffix used to rename the converted triangular elements
  const SubdomainName _tri_elem_subdomain_name_suffix;
  /// Customized id shift to define subdomain ids of the converted triangular elements
  const subdomain_id_type _tri_elem_subdomain_shift;
  /// Reference to input mesh pointer
  std::unique_ptr<MeshBase> & _input;
  /// Number of polygon sides
  unsigned int _num_sides;
  /// MeshMetaData: updated pattern pitch after trimming
  Real & _pattern_pitch_meta;
  /// MeshMetaData: passing input pitch from input
  Real & _input_pitch_meta;
  /// MeshMetaData: passing control drum flag from input
  bool & _is_control_drum_meta;

  /**
   * Performs center trimming on the input mesh.
   * @param mesh input mesh to trim
   * @param num_sides number of polygon sides
   * @param center_trim_sector_number number of sectors to be left after center trimming
   * @param trimming_start_sector index of the first sector to be left
   * @param external_boundary_id ID of the external boundary of the input mesh
   * @param center_trimming_section_boundary_id ID of the new external boundary formed due to
   * trimming
   * @param subdomain_ids_set all the subdomain ids in the input mesh
   */
  void centerTrimmer(ReplicatedMesh & mesh,
                     const unsigned int num_sides,
                     const unsigned int center_trim_sector_number,
                     const unsigned int trimming_start_sector,
                     const boundary_id_type external_boundary_id,
                     const boundary_id_type center_trimming_section_boundary_id,
                     const std::set<subdomain_id_type> subdomain_ids_set);

  /**
   * Performs peripheral trimming on the input mesh.
   * @param mesh input mesh to trim
   * @param trim_peripheral_region whether each of the six sides of peripheral region need to be
   * trimmed
   * @param external_boundary_id ID of the external boundary of the input mesh
   * @param peripheral_trimming_section_boundary_id ID of the new external boundary formed due to
   * trimming
   * @param subdomain_ids_set all the subdomain ids in the input mesh
   */
  void peripheralTrimmer(ReplicatedMesh & mesh,
                         const std::vector<unsigned short> trim_peripheral_region,
                         const boundary_id_type external_boundary_id,
                         const boundary_id_type peripheral_trimming_section_boundary_id,
                         const std::set<subdomain_id_type> subdomain_ids_set);

  /**
   * Removes all the elements on one side of a given line and deforms the elements intercepted by
   * the line to form a flat new boundary
   * @param mesh input mesh to perform line-based elements removing on
   * @param bdry_pars line parameter sets {a, b, c} as in a*x+b*y+c=0
   * @param block_id_to_remove subdomain id used to mark the elements that need to be removed
   * @param subdomain_ids_set all the subdomain ids in the input mesh
   * @param trimming_section_boundary_id ID of the new external boundary formed due to
   * trimming
   * @param external_boundary_id ID of the external boundary of the input mesh
   * @param assign_ext_to_trim whether to assign external_boundary_id to the new boundary formed by
   * removal
   * @param side_to_remove which side of the mesh needs to be removed: true means ax+by+c>0 and
   * false means ax+by+c<0
   */
  void lineRemover(ReplicatedMesh & mesh,
                   const std::vector<Real> bdry_pars,
                   const subdomain_id_type block_id_to_remove,
                   const std::set<subdomain_id_type> subdomain_ids_set,
                   const boundary_id_type trimming_section_boundary_id,
                   const boundary_id_type external_boundary_id = OUTER_SIDESET_ID,
                   const bool assign_ext_to_new = false,
                   const bool side_to_remove = true);

  /**
   * Determines whether a point on XY-plane is on the side of a given line that needs to be removed
   * @param px x coordinate of the point
   * @param py y coordinate of the point
   * @param param_1 parameter 1 (a) in line formula a*x+b*y+c=0
   * @param param_2 parameter 2 (b) in line formula a*x+b*y+c=0
   * @param param_3 parameter 3 (c) in line formula a*x+b*y+c=0
   * @param direction_param which side is the side that needs to be removed
   * @param dis_tol tolerance used in determining side
   * @return whether the point is on the side of the line that needed to be removed
   */
  bool lineSideDeterminator(const Real px,
                            const Real py,
                            const Real param_1,
                            const Real param_2,
                            const Real param_3,
                            const bool direction_param,
                            const Real dis_tol = 1.0E-6);

  /**
   * Calculates the intersection Point of two given straight lines
   * @param param_11 parameter 1 (a) in line formula a*x+b*y+c=0 for the first line
   * @param param_12 parameter 2 (b) in line formula a*x+b*y+c=0 for the first line
   * @param param_13 parameter 3 (c) in line formula a*x+b*y+c=0 for the first line
   * @param param_21 parameter 1 (a) in line formula a*x+b*y+c=0 for the second line
   * @param param_22 parameter 2 (b) in line formula a*x+b*y+c=0 for the second line
   * @param param_23 parameter 3 (c) in line formula a*x+b*y+c=0 for the second line
   * @return intersect point of the two lines
   */
  Point twoLineIntersection(const Real param_11,
                            const Real param_12,
                            const Real param_13,
                            const Real param_21,
                            const Real param_22,
                            const Real param_23);

  /**
   * Fixes degenerate QUAD elements created by the hexagonal mesh trimming by converting them into
   * TRI elements
   * @param mesh input mesh with degenerate QUAD elements that need to be fixed
   * @param subdomain_ids_set all the subdomain ids in the input mesh
   * @return whether any elements have been fixed
   */
  bool quasiTriElementsFixer(ReplicatedMesh & mesh,
                             const std::set<subdomain_id_type> subdomain_ids_set);

  /**
   * Calculates the internal angles of a given 2D element
   * @param elem the element that needs to be investigated
   * @return sizes of all the internal angles, sorted by their size
   */
  std::vector<std::pair<Real, unsigned int>> vertex_angles(Elem & elem) const;
};
