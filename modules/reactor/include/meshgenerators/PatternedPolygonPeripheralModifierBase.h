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
#include "MeshMetaDataInterface.h"
#include "LinearInterpolation.h"

/**
 * This PatternedPolygonPeripheralModifierBase is the base class for
 * PatternedCartesianPeripheralModifier and PatternedHexPeripheralModifier, which remove the outmost
 * layer of the input cartesian/hexagonal mesh and add a transition layer mesh to facilitate
 * stitching.
 */
class PatternedPolygonPeripheralModifierBase : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  PatternedPolygonPeripheralModifierBase(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Name of the input mesh that needs the modification
  const MeshGeneratorName _input_name;
  /// External boundary name of the input mesh
  const BoundaryName _input_mesh_external_boundary;
  /// Target number of mesh sectors on each side of the square
  const unsigned int _new_num_sector;
  /// Block ID of the transition layer to be generated
  const subdomain_id_type _transition_layer_id;
  /// Block name of the transition layer to be generated
  const SubdomainName _transition_layer_name;
  /// Number of element layers of the transition layer to be generated
  const unsigned int _num_layers;
  /// Names of extra element integers in the input mesh that need to be retained or reassigned in the transition layer
  const std::vector<std::string> _extra_id_names_to_modify;
  /// Customized values to be used to reassign the extra element integers in the transition layer
  const std::vector<dof_id_type> _new_extra_id_values_to_assign;
  /// The main mesh used in this class
  std::unique_ptr<MeshBase> & _mesh;
  /// Boundary ID of the external boundary of the input mesh
  boundary_id_type _input_mesh_external_bid;
  /// Number of sides of the mesh to be generated
  unsigned int _num_sides;

  /**
   * Assign extra element integers to the newly generated transition layer mesh based on the
   * nearest element of the deleted original mesh
   * @param mesh the mesh that will be modified to add extra element integer values
   * @param ref_extra_ids a data structure containing the extra element integer information of the
   * deleted original mesh
   */
  void transferExtraElemIntegers(
      ReplicatedMesh & mesh,
      const std::vector<std::pair<Point, std::vector<dof_id_type>>> ref_extra_ids);
};
