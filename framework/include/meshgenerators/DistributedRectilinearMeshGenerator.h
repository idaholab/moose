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

/**
 * Mesh generated from parameters
 */
class DistributedRectilinearMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  DistributedRectilinearMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  template <typename T>
  void build_cube(UnstructuredMesh & mesh,
                  const unsigned int nx,
                  unsigned int ny,
                  unsigned int nz,
                  const Real xmin,
                  const Real xmax,
                  const Real ymin,
                  const Real ymax,
                  const Real zmin,
                  const Real zmax,
                  const ElemType type,
                  bool verbose);

  /**
   * Get the element ID for a given hex
   *
   * @param nx The number of elements in the x direction
   * @param ny The number of elements in the y direction
   * @param i The x index of this element
   * @param j The y index of this element
   * @param k The z index of this element
   * @return The ID of the i,j element
   */

  template <typename T>
  inline dof_id_type elem_id(const dof_id_type /*nx*/,
                             const dof_id_type /*ny*/,
                             const dof_id_type /*i*/,
                             const dof_id_type /*j*/,
                             const dof_id_type /*k*/)
  {
    mooseError(
        "elem_id not implemented for this element type in DistributedRectilinearMeshGenerator");
  }

  /**
   * Get the number of neighbors this element will have
   *
   * @param nx The number of elements in the x direction
   * @param ny The number of elements in the y direction
   * @param nz The number of elements in the z direction
   * @param i The x index of this element
   * @param j The y index of this element
   * @param k The z index of this element
   * @return The number of neighboring elements
   */
  template <typename T>
  inline dof_id_type num_neighbors(const dof_id_type /*nx*/,
                                   const dof_id_type /*ny*/,
                                   const dof_id_type /*nz*/,
                                   const dof_id_type /*i*/,
                                   const dof_id_type /*j*/,
                                   const dof_id_type /*k*/)
  {
    mooseError("num_neighbors not implemented for this element type in "
               "DistributedRectilinearMeshGenerator");
  }

  /**
   * Get the IDs of the neighbors of a given element
   *
   * @param nx The number of elements in the x direction
   * @param nx The number of elements in the y direction
   * @param nz The number of elements in the z direction
   * @param i The x index of this element
   * @param j The y index of this element
   * @param k The z index of this element
   * @param neighbors This will be filled with the IDs of the two neighbors or invalid_dof_id if
   * there is no neighbor.  THIS MUST be of size 6 BEFORE calling this function
   */
  template <typename T>
  inline void get_neighbors(const dof_id_type /*nx*/,
                            const dof_id_type /*ny*/,
                            const dof_id_type /*nz*/,
                            const dof_id_type /*i*/,
                            const dof_id_type /*j*/,
                            const dof_id_type /*k*/,
                            std::vector<dof_id_type> & /*neighbors*/,
                            const bool corner = false);

  /**
   * The ID of the i,j,k node
   *
   * @param type The element type
   * @param nx The number of elements in the x direction
   * @param nx The number of elements in the y direction
   * @param nz The number of elements in the z direction
   * @param i The x index of this node
   * @param j The y index of this node
   * @param k The z index of this node
   */
  template <typename T>
  inline dof_id_type node_id(const ElemType /*type*/,
                             const dof_id_type /*nx*/,
                             const dof_id_type /*ny*/,
                             const dof_id_type /*i*/,
                             const dof_id_type /*j*/,
                             const dof_id_type /*k*/)
  {
    mooseError(
        "node_id not implemented for this element type in DistributedRectilinearMeshGenerator");
  }

  /**
   * Add a node to the mesh
   *
   * @param nx The number of elements in the x direction
   * @param nx The number of elements in the y direction
   * @param nz The number of elements in the z direction
   * @param i The x index of this node
   * @param j The y index of this node
   * @param k The z index of this node
   * @param type The element type
   * @param mesh The mesh to add it to
   */
  template <typename T>
  Node * add_point(const dof_id_type /*nx*/,
                   const dof_id_type /*ny*/,
                   const dof_id_type /*nz*/,
                   const dof_id_type /*i*/,
                   const dof_id_type /*j*/,
                   const dof_id_type /*k*/,
                   const ElemType /*type*/,
                   MeshBase & /*mesh*/)
  {
    mooseError(
        "add_point not implemented for this element type in DistributedRectilinearMeshGenerator");
  }

  /**
   * Adds an element to the mesh
   *
   * @param nx The number of elements in the x direction
   * @param ny The number of elements in the y direction
   * @param nz The number of elements in the z direction
   * @param i The x index of this element
   * @param j The y index of this element
   * @param k The z index of this element
   * @param elem_id The element ID of the element to add
   * @param pid The processor ID to assign it to
   * @param type The type of element to add
   * @param mesh The mesh to add it to
   * @param verbose Whether or not to print out verbose statements
   */
  template <typename T>
  void add_element(const dof_id_type /*nx*/,
                   const dof_id_type /*ny*/,
                   const dof_id_type /*nz*/,
                   const dof_id_type /*i*/,
                   const dof_id_type /*j*/,
                   const dof_id_type /*k*/,
                   const dof_id_type /*elem_id*/,
                   const processor_id_type /*pid*/,
                   const ElemType /*type*/,
                   MeshBase & /*mesh*/,
                   bool /*verbose*/)
  {
    mooseError(
        "add_element not implemented for this element type in DistributedRectilinearMeshGenerator");
  }

  /**
   * Compute the i,j,k indices of a given element ID
   *
   * @param nx The number of elements in the x direction
   * @param ny The number of elements in the y direction
   * @param elem_id The ID of the element
   * @param i Output: The index in the x direction
   * @param j Output: The index in the y direction
   * @param k Output: The index in the z direction
   */
  template <typename T>
  inline void get_indices(const dof_id_type /*nx*/,
                          const dof_id_type /*ny*/,
                          const dof_id_type /*elem_id*/,
                          dof_id_type & /*i*/,
                          dof_id_type & /*j*/,
                          dof_id_type & /*k*/)
  {
    mooseError(
        "get_indices not implemented for this element type in DistributedRectilinearMeshGenerator");
  }

  /**
   * Find the elements and sides that need ghost elements
   *
   * @param nx The number of elements in the x direction
   * @param ny The number of elements in the y direction
   * @param mesh The mesh - without any ghost elements
   * @param ghost_elems The ghost elems that need to be added
   */
  template <typename T>
  inline void get_ghost_neighbors(const dof_id_type /*nx*/,
                                  const dof_id_type /*ny*/,
                                  const dof_id_type /*nz*/,
                                  const MeshBase & /*mesh*/,
                                  std::set<dof_id_type> & /*ghost_elems*/)
  {
    mooseError("get_ghost_neighbors not implemented for this element type in "
               "DistributedRectilinearMeshGenerator");
  }

  /**
   * Set the boundary names for any added boundary ideas
   *
   * @boundary_info The BoundaryInfo object to set the boundary names on
   */
  template <typename T>
  void set_boundary_names(BoundaryInfo & /*boundary_info*/)
  {
    mooseError("set_boundary_names not implemented for this element type in "
               "DistributedRectilinearMeshGenerator");
  }

  /**
   * All meshes are generated on the unit square.  This function stretches the mesh
   * out to fill the correct area.
   */
  template <typename T>
  void scale_nodal_positions(dof_id_type /*nx*/,
                             dof_id_type /*ny*/,
                             dof_id_type /*nz*/,
                             Real /*xmin*/,
                             Real /*xmax*/,
                             Real /*ymin*/,
                             Real /*ymax*/,
                             Real /*zmin*/,
                             Real /*zmax*/,
                             MeshBase & /*mesh*/)
  {
    mooseError("scale_nodal_positions not implemented for this element type in "
               "DistributedRectilinearMeshGenerator");
  }

protected:
  /// If to print debug information
  bool _verbose;

  /// The dimension of the mesh
  MooseEnum _dim;

  /// Number of elements in x, y, z direction
  dof_id_type &_nx, &_ny, &_nz;

  /// The min/max values for x,y,z component
  Real &_xmin, &_xmax, &_ymin, &_ymax, &_zmin, &_zmax;

  /// The type of element to build
  ElemType _elem_type;

  /// The amount by which to bias the cells in the x,y,z directions.
  /// Must be in the range 0.5 <= _bias_x <= 2.0.
  /// _bias_x < 1 implies cells are shrinking in the x-direction.
  /// _bias_x==1 implies no bias (original mesh unchanged).
  /// _bias_x > 1 implies cells are growing in the x-direction.
  Real _bias_x, _bias_y, _bias_z;

  /// External partitioner
  std::string _part_package;

  /// Number of cores per compute node if hierarch partitioning is used
  dof_id_type _num_parts_per_compute_node;
};
