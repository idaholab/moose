//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "libmesh/elem.h"

namespace RGMB
{

// General global quantities for mesh building
static const std::string mesh_dimensions = "mesh_dimensions";
static const std::string mesh_geometry = "mesh_geometry";
static const std::string top_boundary_id = "top_boundary_id";
static const std::string bottom_boundary_id = "bottom_boundary_id";
static const std::string radial_boundary_id = "radial_boundary_id";
static const std::string axial_mesh_intervals = "axial_mesh_intervals";
static const std::string axial_mesh_sizes = "axial_mesh_sizes";
static const std::string reactor_params_name = "reactor_params_name";
static const std::string is_single_pin = "is_single_pin";
static const std::string is_homogenized = "is_homogenized";
static const std::string extruded = "extruded";
static const std::string pin_region_ids = "pin_region_ids";
static const std::string pin_block_names = "pin_block_names";
static const std::string pin_region_id_map = "pin_region_id_map";
static const std::string pin_block_name_map = "pin_block_name_map";
static const std::string flexible_assembly_stitching = "flexible_assembly_stitching";
static const std::string num_sectors_flexible_stitching = "num_sectors_flexible_stitching";
static const std::string is_control_drum = "is_control_drum";
static const std::string drum_region_ids = "drum_region_ids";
static const std::string drum_block_names = "drum_block_names";

// Geometrical quantities
static const std::string pitch = "pitch";
static const std::string assembly_pitch = "assembly_pitch";
static const std::string ring_radii = "ring_radii";
static const std::string duct_halfpitches = "duct_halfpitches";
static const std::string peripheral_ring_radius = "peripheral_ring_radius";
static const std::string pin_lattice = "pin_lattice";
static const std::string assembly_lattice = "assembly_lattice";
static const std::string drum_pad_angles = "drum_pad_angles";
static const std::string drum_radii = "drum_radii";

// Quantities related to region ids, type ids, and block names
static const std::string pin_type = "pin_type";
static const std::string pin_names = "pin_names";
static const std::string assembly_type = "assembly_type";
static const std::string assembly_names = "assembly_names";
static const std::string ring_region_ids = "ring_region_ids";
static const std::string background_region_id = "background_region_id";
static const std::string background_block_name = "background_block_name";
static const std::string duct_region_ids = "duct_region_ids";
static const std::string duct_block_names = "duct_block_names";
static const std::string peripheral_ring_region_id = "peripheral_ring_region_id";
static const std::string region_id_as_block_name = "region_id_as_block_name";

// Name of a boolean metadata that indicates whether or not we skipped mesh generation in favor of
// only generating the mesh metadata
static const std::string bypass_meshgen = "bypass_meshgen";

// Default values for setting block IDs and region IDs of RGMB regions
const subdomain_id_type PIN_BLOCK_ID_TRI_FLEXIBLE = 9998;
const subdomain_id_type PIN_BLOCK_ID_TRI = 9999;
const subdomain_id_type PIN_BLOCK_ID_START = 10000;

const subdomain_id_type CONTROL_DRUM_BLOCK_ID_INNER_TRI = 19995;
const subdomain_id_type CONTROL_DRUM_BLOCK_ID_INNER = 19996;
const subdomain_id_type CONTROL_DRUM_BLOCK_ID_PAD = 19997;
const subdomain_id_type CONTROL_DRUM_BLOCK_ID_OUTER = 19998;

const subdomain_id_type ASSEMBLY_BLOCK_ID_TRI_FLEXIBLE = 19999;
const subdomain_id_type ASSEMBLY_BLOCK_ID_START = 20000;

const subdomain_id_type DUMMY_ASSEMBLY_BLOCK_ID = (UINT16_MAX / 2) - 1;
const subdomain_id_type PERIPHERAL_RING_BLOCK_ID = 25000;

const subdomain_id_type MAX_PIN_TYPE_ID = (UINT16_MAX / 2) - 1;

// Default values for setting block names of RGMB regions
const SubdomainName PIN_BLOCK_NAME_PREFIX = "RGMB_PIN";
const SubdomainName ASSEMBLY_BLOCK_NAME_PREFIX = "RGMB_ASSEMBLY";
const SubdomainName DRUM_BLOCK_NAME_PREFIX = "RGMB_DRUM";
const SubdomainName CORE_BLOCK_NAME_PREFIX = "RGMB_CORE";
const SubdomainName TRI_BLOCK_NAME_SUFFIX = "_TRI";
const SubdomainName PERIPHERAL_RING_BLOCK_NAME = "PERIPHERY_GENERATED";

// Default values for setting boundary ids of RGMB regions
static constexpr boundary_id_type PIN_BOUNDARY_ID_START = 20000;
static constexpr boundary_id_type ASSEMBLY_BOUNDARY_ID_START = 2000;

// Default values for setting boundary names of RGMB regions
const BoundaryName PIN_BOUNDARY_NAME_PREFIX = "outer_pin_";
const BoundaryName ASSEMBLY_BOUNDARY_NAME_PREFIX = "outer_assembly_";
const BoundaryName CORE_BOUNDARY_NAME = "outer_core";
}

/**
 * A base class that contains common members for Reactor Geometry Mesh Builder mesh generators.
 */
class ReactorGeometryMeshBuilderBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  static void addDepletionIDParams(InputParameters & parameters);

  ReactorGeometryMeshBuilderBase(const InputParameters & parameters);

  void generateData() override{};

protected:
  /**
   * Initializes extra element integer from id name for a given mesh and throws an error
   * if it should exist but cannot be found within the mesh
   * @param input_mesh input mesh
   * @param extra_int_name extra element id name
   * @param should_exist whether extra element integer should already exist in mesh
   * @return extra element integer
   */
  unsigned int getElemIntegerFromMesh(MeshBase & input_mesh,
                                      std::string extra_int_name,
                                      bool should_exist = false);

  /**
   * Initializes and checks validity of ReactorMeshParams mesh generator object
   * @param reactor_param_name name of ReactorMeshParams mesh generator
   */
  void initializeReactorMeshParams(const std::string reactor_param_name);

  /**
   * Print metadata associated with ReactorGeometryMeshBuilder object
   * @param geometry_type       type of geometry (pin / assembly / core) under consideration
   * @param mg_name             name of mesh generator associated with this object
   * @param first_function_call whether this is the original function call, which will trigger
   * additional output messages
   */
  void printReactorMetadata(const std::string geometry_type,
                            const std::string mg_name,
                            const bool first_function_call = true);

  /**
   * Print core-level metadata associated with ReactorGeometryMeshBuilder object
   * @param mg_name name of mesh generator associated with core
   * @param first_function_call whether this is the original function call, which will trigger
   * additional output messages
   */
  void printCoreMetadata(const std::string mg_name, const bool first_function_call);

  /**
   * Print assembly-level metadata associated with ReactorGeometryMeshBuilder object
   * @param mg_name name of mesh generator associated with assembly
   * @param whether this is the original function call, which will trigger additional output
   * messages
   */
  void printAssemblyMetadata(const std::string mg_name, const bool first_function_call);

  /**
   * Print pin-level metadata associated with ReactorGeometryMeshBuilder object
   * @param mg_name name of mesh generator associated with assembly
   */
  void printPinMetadata(const std::string mg_name);

  /**
   * Print global ReactorMeshParams metadata associated with ReactorGeometryMeshBuilder object
   */
  void printGlobalReactorMetadata();

  /**
   * Print metadata with provided name that can be found with given mesh generator name
   * @tparam T datatype of metadata value to output
   * @param metadata_name Name of metadata to output
   * @param mg_name Name of mesh generator that stores metadata
   */
  template <typename T>
  void printMetadataToConsole(const std::string metadata_name, const std::string mg_name);

  /**
   * Print metadata with data type std::vector<std::vector<T>> and provided name that can be found
   * with given mesh generator name
   * @tparam T datatype of elements in 2-D vector to output
   * @param metadata_name Name of metadata to output
   * @param mg_name Name of mesh generator that stores metadata
   */
  template <typename T>
  void print2dMetadataToConsole(const std::string metadata_name, const std::string mg_name);

  /**
   * Releases the mesh obtained in _reactor_params_mesh.
   *
   * This _must_ be called in any object that derives from this one, because
   * the MeshGenerator system requires that all meshes that are requested from
   * the system are moved out of the MeshGenerator system and into the MeshGenerator
   * that requests them. In our case, we move it into this MeshGenerator and then
   * release (delete) it.
   */
  void freeReactorMeshParams();

  /**
   * Checks whether parameter is defined in ReactorMeshParams metadata
   * @tparam T datatype of metadata value associated with metadata name
   * @param param_name name of ReactorMeshParams parameter
   * @return whether parameter is defined in ReactorMeshParams metadata
   */
  template <typename T>
  bool hasReactorParam(const std::string param_name);

  /**
   * Returns reference of parameter in ReactorMeshParams object
   * @tparam T datatype of metadata value associated with metadata name
   * @param param_name name of ReactorMeshParams parameter
   * @return reference to parameter defined in ReactorMeshParams metadata
   */
  template <typename T>
  const T & getReactorParam(const std::string & param_name);

  /**
   * Updates the block names and ids of the element in an input mesh according
   * to a map of block name to block ids. Updates the map if the block name is not in the map
   * @param input_name input mesh
   * @param elem iterator to mesh element
   * @param name_id_map map of name-id pairs used in mesh
   * @param elem_block_name block name to set for element
   * @param next_free_id next free block id to use if block name does not exist in map
   */
  void updateElementBlockNameId(MeshBase & input_mesh,
                                Elem * elem,
                                std::map<std::string, SubdomainID> & name_id_map,
                                std::string elem_block_name,
                                SubdomainID & next_free_id);

  /**
   * Calls mesh subgenerators related to extrusion, renaming of top / bottom boundaries, and
   * defining plane IDs
   * @param input_mesh_name name of input 2D mesh generator to extrude
   * @return name of final output 3D mesh generator
   */
  MeshGeneratorName callExtrusionMeshSubgenerators(const MeshGeneratorName input_mesh_name);

  ///The ReactorMeshParams object that is storing the reactor global information for this reactor geometry mesh
  MeshGeneratorName _reactor_params;
  /// specify the depletion id is generated at which reactor generation level
  enum class DepletionIDGenerationLevel
  {
    Pin,
    Assembly,
    Drum,
    Core
  };

  /**
   * add depletion IDs
   * @param input_mesh input mesh
   * @param option option for specifying level of details
   * @param generation_level depletion id is generated at which reactor generator level
   * @param extrude whether input mesh is extruded, if false, assume that input mesh is defined in
   * 2D and do not use 'plane_id` in depletion id generation
   */
  void addDepletionId(MeshBase & input_mesh,
                      const MooseEnum & option,
                      const DepletionIDGenerationLevel generation_level,
                      const bool extrude);

private:
  /// The dummy param mesh that we need to clear once we've generated (in freeReactorMeshParams)
  std::unique_ptr<MeshBase> * _reactor_params_mesh;
};

template <typename T>
bool
ReactorGeometryMeshBuilderBase::hasReactorParam(const std::string param_name)
{
  return hasMeshProperty<T>(param_name, _reactor_params);
}

template <typename T>
const T &
ReactorGeometryMeshBuilderBase::getReactorParam(const std::string & param_name)
{
  return getMeshProperty<T>(param_name, _reactor_params);
}
