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
#include "MooseEnum.h"
#include "CastUniquePointer.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_tri3.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/boundary_info.h"
#include "libmesh/utility.h"

/**
 * A base class that contains common members for Reactor module mesh generators.
 */

class PolygonMeshGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  PolygonMeshGeneratorBase(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

  enum MESH_TYPE
  {
    CORNER_MESH = 1,
    BOUNDARY_MESH = 2
  };

  enum RETURN_TYPE
  {
    ANGLE_DEGREE = 1,
    ANGLE_TANGENT = 2
  };

  enum INTRISIC_SUBDOMAIN_ID : subdomain_id_type
  {
    PERIPHERAL_ID_SHIFT = 1000
  };
  // boundary_id_type is short int so max is 32,767
  enum INTRINSIC_SIDESET_ID : boundary_id_type
  {
    OUTER_SIDESET_ID = 10000,
    OUTER_SIDESET_ID_ALT = 15000,
    SLICE_BEGIN = 30000,
    SLICE_END = 31000,
    SLICE_ALT = 30500
  };

  enum INTRINSIC_NUM_SIDES
  {
    HEXAGON_NUM_SIDES = 6
  };

protected:
  // 'build_simple_slice' creates a mesh of a slice that corresponds to a single side of the polygon
  // to be generated.
  std::unique_ptr<ReplicatedMesh>
  build_simple_slice(std::unique_ptr<ReplicatedMesh> mesh,
                     std::vector<Real> ring_radii,
                     std::vector<unsigned int> rings,
                     std::vector<Real> ducts_center_dist,
                     std::vector<unsigned int> ducts_layers,
                     bool has_rings,
                     bool has_ducts,
                     Real pitch,
                     unsigned int num_sectors_per_side,
                     unsigned int background_intervals,
                     dof_id_type * node_id_background_meta,
                     const unsigned int side_number,
                     const unsigned int side_index,
                     const std::vector<Real> azimuthal_tangent = std::vector<Real>(),
                     const unsigned int block_id_shift = 0,
                     const bool quad_center_elements = false,
                     const unsigned int boundary_id_shift = 0);

  // 'center_nodes' create nodes of the very central mesh layer of the polygon for quad central
  // elements
  std::unique_ptr<ReplicatedMesh> center_nodes(std::unique_ptr<ReplicatedMesh> mesh,
                                               const unsigned int side_number,
                                               const unsigned int div_num,
                                               const Real ring_radii_0,
                                               dof_id_type * node_id,
                                               std::vector<Node *> * nodes,
                                               std::vector<std::vector<dof_id_type>> * id_array);

  // 'ring_nodes' creates nodes for the ring-geometry region of a single slice.
  std::unique_ptr<ReplicatedMesh>
  ring_nodes(std::unique_ptr<ReplicatedMesh> mesh,
             std::vector<Real> ring_radii,
             const std::vector<unsigned int> rings,
             const unsigned int num_sectors_per_side,
             dof_id_type * node_id,
             const Real corner_p[2][2],
             const Real corner_to_corner,
             std::vector<Node *> * nodes,
             const std::vector<Real> azimuthal_tangent = std::vector<Real>());

  // 'background_nodes' creates nodes for the ring-to-polygon transition region (i.e., background)
  // of a single slice.
  std::unique_ptr<ReplicatedMesh>
  background_nodes(std::unique_ptr<ReplicatedMesh> mesh,
                   const unsigned int num_sectors_per_side,
                   const unsigned int background_intervals,
                   const Real background_corner_distance,
                   const Real background_corner_radial_interval_length,
                   dof_id_type * node_id,
                   const Real corner_p[2][2],
                   const Real corner_to_corner,
                   std::vector<Node *> * nodes,
                   const Real background_in,
                   const std::vector<Real> azimuthal_tangent = std::vector<Real>());

  // 'duct_nodes' creates nodes for the duct-geometry region of a single slice.
  std::unique_ptr<ReplicatedMesh>
  duct_nodes(std::unique_ptr<ReplicatedMesh> mesh,
             std::vector<Real> * ducts_center_dist,
             const std::vector<unsigned int> ducts_layers,
             const unsigned int num_sectors_per_side,
             dof_id_type * node_id,
             const Real corner_p[2][2],
             const Real corner_to_corner,
             std::vector<Node *> * nodes,
             const std::vector<Real> azimuthal_tangent = std::vector<Real>());

  // 'cen_quad_elem_def' defines quad elements in the very central region of the polygon.
  std::unique_ptr<ReplicatedMesh>
  cen_quad_elem_def(std::unique_ptr<ReplicatedMesh> mesh,
                    const unsigned int div_num,
                    const std::vector<Node *> nodes,
                    const unsigned int block_id_shift,
                    const unsigned int boundary_id_shift,
                    std::vector<std::vector<dof_id_type>> * id_array);

  // 'cen_tri_elem_def' defines triangular elements in the very central region of the polygon.
  std::unique_ptr<ReplicatedMesh>
  cen_tri_elem_def(std::unique_ptr<ReplicatedMesh> mesh,
                   const unsigned int num_sectors_per_side,
                   const std::vector<Node *> nodes,
                   const std::vector<Real> azimuthal_tangent = std::vector<Real>(),
                   const unsigned int block_id_shift = 0,
                   const unsigned int boundary_id_shift = 0);

  // 'quad_elem_def' defines general quad elements for the polygon.
  std::unique_ptr<ReplicatedMesh>
  quad_elem_def(std::unique_ptr<ReplicatedMesh> mesh,
                const unsigned int num_sectors_per_side,
                const std::vector<unsigned int> subdomain_rings,
                const std::vector<Node *> nodes,
                const unsigned int side_index,
                const std::vector<Real> azimuthal_tangent = std::vector<Real>(),
                const unsigned int block_id_shift = 0,
                const unsigned int nodeid_shift = 0,
                const unsigned int boundary_id_shift = 0);

  // 'build_simple_peripheral' creates peripheral area mesh for the patterned hexagon mesh
  // Note that the function creat the pheripheral area for each side of the unit hexagon mesh before
  // stitching. An edge unit hexagon has two sides that need peripheral areas, whereas a corner unit
  // hexagon has three such sides. The positions of the inner and outer boundary nodes are
  // pre-calculated as positions_inner and d_positions_outer; This function performs interpolation
  // to generate the mesh grid.
  std::unique_ptr<ReplicatedMesh>
  build_simple_peripheral(std::unique_ptr<ReplicatedMesh> mesh,
                          const unsigned int num_sectors_per_side,
                          const unsigned int peripheral_invervals,
                          const std::vector<std::pair<Real, Real>> position_inner,
                          const std::vector<std::pair<Real, Real>> d_position_outer,
                          std::vector<Node *> * nodes,
                          const unsigned int id_shift);

  // 'radius_correction_factor' makes radial correction to preserve ring area.
  Real radius_correction_factor(const std::vector<Real> azimuthal_list);

  // 'point_interpolate' calculates the point coordinates of within a parallelogram region using
  // linear interpolation.
  std::pair<Real, Real> point_interpolate(const Real pi_1_x,
                                          const Real pi_1_y,
                                          const Real po_1_x,
                                          const Real po_1_y,
                                          const Real pi_2_x,
                                          const Real pi_2_y,
                                          const Real po_2_x,
                                          const Real po_2_y,
                                          const unsigned int i,
                                          const unsigned int j,
                                          const unsigned int num_sectors_per_side,
                                          const unsigned int peripheral_invervals);

  // 'add_peripheral_mesh' adds background and duct region mesh to stiched hexagon meshes.
  // Note that the function works for single unit hexagon mesh (corner or edge) separately before
  // stitching
  std::unique_ptr<ReplicatedMesh>
  add_peripheral_mesh(const unsigned int pattern, //_pattern{i][j]
                      const Real pitch,           // pitch_array.front()
                      const std::vector<Real> extra_dist,
                      const std::unique_ptr<ReplicatedMesh> * out_meshes,
                      const std::vector<unsigned int> num_sectors_per_side_array,
                      const std::vector<unsigned int> peripheral_duct_intervals,
                      const Real rotation_angle,
                      const unsigned int mesh_type);

  // 'position_setup' sets up poisitions of peripheral region before deformation due to cutoff.
  // Nine sets of positions are generated here, as shown below.
  void position_setup(std::vector<std::pair<Real, Real>> * positions_inner,
                      std::vector<std::pair<Real, Real>> * d_positions_outer,
                      const Real extra_dist_in,
                      const Real extra_dist_out,
                      const Real pitch,
                      const unsigned int radial_index);

  // 'node_coord_rotate' calculates x and y coordinates after rotating by theta angle.
  void node_coord_rotate(Real * x, Real * y, const Real theta);

  // 'cut_off_hex_deform' deforms peripheral region when the external side of a hexagonal assembly
  // of stitched meshes cuts off the stitched meshes.
  void cut_off_hex_deform(MeshBase & mesh,
                          const Real orientation,
                          const Real y_max_0,
                          const Real y_max_n,
                          const Real y_min,
                          const unsigned int mesh_type,
                          const Real tols = 1E-5);

  // 'four_point_intercept' finds the center of a quadrilateral based on four vertices.
  std::pair<Real, Real> four_point_intercept(const std::pair<Real, Real> p1,
                                             const std::pair<Real, Real> p2,
                                             const std::pair<Real, Real> p3,
                                             const std::pair<Real, Real> p4);

  // 'azimuthal_angles_collector' collects sorted azimuthal angles of the external boundary.
  std::vector<Real> azimuthal_angles_collector(const std::unique_ptr<ReplicatedMesh> mesh,
                                               const Real lower_azi = -30.0,
                                               const Real upper_azi = 30.0,
                                               const unsigned int return_type = ANGLE_TANGENT,
                                               const bool calculate_origin = false,
                                               const Real input_origin_x = 0.0,
                                               const Real input_origin_y = 0.0,
                                               const Real tol = 1.0E-10);
};
