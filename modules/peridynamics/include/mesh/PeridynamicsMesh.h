//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseApp.h"

#include "libmesh/point.h"

/**
 * Peridynamics mesh class
 */
class PeridynamicsMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  PeridynamicsMesh(const InputParameters & parameters);
  PeridynamicsMesh(const PeridynamicsMesh & /* other_mesh */) = default;

  PeridynamicsMesh & operator=(const PeridynamicsMesh & other_mesh) = delete;
  virtual std::unique_ptr<MooseMesh> safeClone() const override;
  virtual void buildMesh() override;
  virtual unsigned int dimension() const override;

  /**
   * Function to return number of PD nodes
   * @return Total number of PD nodes
   */
  dof_id_type nPDNodes() const;

  /**
   * Function to return number of PD Edge elements
   * @return Total number of PD bonds
   */
  dof_id_type nPDBonds() const;

  /**
   * Function to assign values to member variables (PD mesh data) of this class
   * this function will be called in the PD mesh generator class
   * @param fe_mesh   The finite element mesh based on which the peridynamics mesh will be created
   * @param converted_elem_id  The IDs of finite elements to be converted to peridynamics mesh
   */
  void createPeridynamicsMeshData(MeshBase & fe_mesh,
                                  std::set<dof_id_type> converted_elem_id,
                                  std::multimap<SubdomainID, SubdomainID> bonding_block_pairs,
                                  std::multimap<SubdomainID, SubdomainID> non_bonding_block_pairs);

  /**
   * Function to return neighbor nodes indices for node node_id
   * @param node_id   The querying node index
   * @return List of neighbor IDs
   */
  std::vector<dof_id_type> getNeighbors(dof_id_type node_id);

  /**
   * Function to return the local neighbor index of node_j from node_i's neighbor list
   * @param node_i   The ID of the node providing the neighbor list for query
   * @param node_j   The ID of the node querying the neighbor index
   * @return The local index in the neighbor list
   */
  dof_id_type getNeighborIndex(dof_id_type node_i, dof_id_type node_j);

  /**
   * Function to return the bond number connected with node node_id
   * @param node_id   The querying node index
   * @return List of associated bond IDs
   */
  std::vector<dof_id_type> getBonds(dof_id_type node_id);

  /**
   * Function to return indices of neighbors used in formulation of bond-associated
   * deformation gradient for bond connecting node_id and neighbor_id
   * @param node_id   The ID of the node providing the neighbor list
   * @param neighbor_id   The ID of the node querying the neighbor list
   * @return The neighbor list for calculation of bond-associated deformation gradient for the
   * neighbor
   */
  std::vector<dof_id_type> getBondDeformationGradientNeighbors(dof_id_type node_id,
                                                               dof_id_type neighbor_id);

  /**
   * Function to return block ID for node node_id
   * @param node_id   The querying node index
   * @return The block ID of a node
   */
  SubdomainID getNodeBlockID(dof_id_type node_id);

  /**
   * Function to set block ID for all PD nodes
   * @param id   The subdomain ID for all PD nodes
   */
  void setNodeBlockID(SubdomainID id);

  /**
   * Function to return coordinates for node node_id
   * @param node_id   The querying node index
   * @return The coordinates of a node
   */
  Point getNodeCoord(dof_id_type node_id);

  /**
   * Function to return the correspondence between PD node IDs and FE element IDs
   * @return The coordinates of a node
   */
  std::vector<dof_id_type> getPDNodeIDToFEElemIDMap();

  /**
   * Function to return nodal volume for node node_id
   * @param node_id   The querying node index
   * @return The volume of a node
   */
  Real getNodeVolume(dof_id_type node_id);

  /**
   * Function to return summation of neighbor nodal volumes for node node_id
   * @param node_id   The querying node index
   * @return The summation of the volume of all the neighbors
   */
  Real getHorizonVolume(dof_id_type node_id);

  /**
   * Function to return the volume of a horizon subset for bond-associated deformation gradient
   * calculation for bond connecting node node_id and its neighbor neighbor_id
   * @param node_id   The ID of the node
   * @param neighbor_id   The ID of the querying neighbor
   * @return The horizon subset volume
   */
  Real getHorizonSubsetVolume(dof_id_type node_id, dof_id_type neighbor_id);

  /**
   * Function to return the summation of all horizon subset volumes for node node_id
   * @param node_id   The querying node index
   * @return The summation of all horizon subset volumes
   */
  Real getHorizonSubsetVolumeSum(dof_id_type node_id);

  /**
   * Function to return the volume fraction of a horizon subset used for bond-associated deformation
   * gradient calculation for bond connecting node node_id and its neighbor neighbor_id
   * @param node_id   The ID of the node
   * @param neighbor_id   The ID of the querying neighbor
   * @return The horizon subset volume fraction for neighbor neighbor_id of node node_id
   */
  Real getHorizonSubsetVolumeFraction(dof_id_type node_id, dof_id_type neighbor_id);

  /**
   * Function to return the average spacing between node node_id with its most adjacent neighbors
   * @param node_id   The querying node index
   * @return The node average spacing
   */
  Real getNodeAverageSpacing(dof_id_type node_id);

  /**
   * Function to return horizon size
   * @param node_id   The querying node index
   * @return The horizon radius
   */
  Real getHorizon(dof_id_type node_id);

  /**
   * Function to return offset for boundary nodes
   * @param node_id   The querying node index
   * @return The offset
   */
  Real getBoundaryOffset(dof_id_type node_id);

protected:
  ///@{ Horizon size control parameters
  const Real _horizon_radius;
  const bool _has_horizon_number;
  const Real _horizon_number;
  const Real _bah_ratio;
  ///@}

  ///@{ Information for crack generation
  const bool _has_cracks;
  std::vector<Point> _cracks_start;
  std::vector<Point> _cracks_end;
  std::vector<Real> _cracks_width;
  ///@}

  /// Mesh dimension
  unsigned int & _dim;

  /// Number of total material points
  unsigned int & _n_pdnodes;

  /// Number of total bonds
  unsigned int & _n_pdbonds;

  ///@{ Data associated with each peridynamics node
  std::vector<Point> _pdnode_coord;
  std::vector<Real> & _pdnode_average_spacing;
  std::vector<Real> & _pdnode_horizon_radius;
  std::vector<Real> & _pdnode_vol;
  std::vector<Real> & _pdnode_horizon_vol;
  std::vector<SubdomainID> & _pdnode_blockID;
  std::vector<dof_id_type> & _pdnode_elemID;
  ///@}

  /// Neighbor lists for each material point determined using the horizon
  std::vector<std::vector<dof_id_type>> & _pdnode_neighbors;

  /// Bond lists associated with material points
  std::vector<std::vector<dof_id_type>> & _pdnode_bonds;

  /// Neighbor lists for deformation gradient calculation using bond-associated horizon
  std::vector<std::vector<std::vector<dof_id_type>>> & _dg_neighbors;

  /// Volume of horizon subsets for bond-associated deformation gradients at a node
  std::vector<std::vector<Real>> & _pdnode_sub_vol;

  /// Summation of volumes of all horizon subsets at a node
  std::vector<Real> & _pdnode_sub_vol_sum;

  /// Offset of each boundary node to its original FE element boundary edge or face
  std::map<dof_id_type, Real> & _boundary_node_offset;

  /**
   * Function to create neighbors and other data for each material point with given horizon
   * @param bonding_block_pairs   ID pairs of blocks to be connected via interfacial bonds
   * @param non_bonding_block_pairs   ID pairs of blocks not to be connected
   */
  void createNodeHorizBasedData(std::multimap<SubdomainID, SubdomainID> bonding_block_pairs,
                                std::multimap<SubdomainID, SubdomainID> non_bonding_block_pairs);

  /**
   * Function to check existence of interface between two blocks
   * @param blockID_i & blockID_j   IDs of two querying blocks
   * @param blockID_pairs   ID pairs of neighboring blocks
   */
  bool checkInterface(SubdomainID pdnode_blockID_i,
                      SubdomainID pdnode_blockID_j,
                      std::multimap<SubdomainID, SubdomainID> blockID_pairs);

  /**
   * Function to create node neighbors and other data for each material point based on
   * NEIGHBOR_HORIZON based horizon partition for deformation gradient calculation
   */
  void createNeighborHorizonBasedData();

  /**
   * Function to check whether a material point falls within a given rectangular crack geometry
   * @param point   The querying point
   * @param rec_p1   The center of one edge of a rectangle
   * @param rec_p2   The center of the opposite edge of the retangle
   * @param rec_height   The distance between the rest two edges of the rectangle
   * @param tol   Acceptable tolerence
   * @return Whether the given point is within the given rectangule
   */
  bool
  checkPointInsideRectangle(Point point, Point rec_p1, Point rec_p2, Real rec_height, Real tol = 0);

  /**
   * Function to check whether a bond crosses crack surface
   * @param crack_p1   Crack starting point
   * @param crack_p2   Crack ending point
   * @param crack_width   Crack width
   * @param bond_p1   Bond starting point
   * @param bond_p2   Bond ending point
   * @return Whether the given bond crosses the given crack surface
   */
  bool checkCrackIntersectBond(
      Point crack_p1, Point crack_p2, Real crack_width, Point bond_p1, Point bond_p2);

  /**
   * Function to check whether a segment crosses another segment
   * @param seg1_p1   The starting point of segment 1
   * @param seg1_p2   The ending point of segment 1
   * @param seg2_p1   The starting point of segment 2
   * @param seg2_p2   The ending point of segment 2
   * @return Whether the given segements cross each other
   */
  bool checkSegmentIntersectSegment(Point seg1_p1, Point seg1_p2, Point seg2_p1, Point seg2_p2);
};
