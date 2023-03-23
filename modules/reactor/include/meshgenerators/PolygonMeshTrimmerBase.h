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
};
