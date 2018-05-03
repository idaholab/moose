//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MESHBASEPD_H
#define MESHBASEPD_H

#include "MooseMesh.h"

class MeshBasePD;

template <>
InputParameters validParams<MeshBasePD>();

/**
 * Structure to store data for each peridynamic material point
 */
struct PD_Node
{
  Point coord;
  Real mesh_spacing;
  Real horizon;
  Real volume;
  Real volumesum;
  unsigned int blockID;
};

/**
 * Base mesh class for peridynamic models
 */
class MeshBasePD : public MooseMesh
{
public:
  MeshBasePD(const InputParameters & parameters);

  virtual unsigned int dimension() const override;
  virtual dof_id_type nNodes() const override;
  virtual dof_id_type nElem() const override;

  /**
   * Function to compute the  horizon size for each node
   */
  Real computeHorizon(Real spacing);

  /**
   * Function for node neighbor search with given horizon
   */
  void findNodeNeighbor();

  /**
   * Function to setup node information for bond deformation gradient calculation
   */
  void setupDGNodeInfo();

  /**
   * Function to return neighbor nodes indices for node node_id
   */
  std::vector<dof_id_type> neighbors(dof_id_type node_id);

  /**
   * Function to return the neighbor index of node_j from node_i's neighbor list
   */
  unsigned int neighborID(dof_id_type node_i, dof_id_type node_j);

  /**
   * Function to return neighbor nodes indices for node node_id
   */
  std::vector<dof_id_type> bonds(dof_id_type node_id);

  /**
   * Function to return nodes indices for calculating deformation gradient
   * for bond between node_id and neighbor_id
   */
  std::vector<unsigned int> dgNodeInfo(dof_id_type node_id, unsigned int neighbor_id);

  /**
   * Function to return coordinates for node node_id
   */
  Point coord(dof_id_type node_id);

  /**
   * Function to return nodal volume for node node_id
   */
  Real volume(dof_id_type node_id);

  /**
   * Function to return summation of neighbor nodal volumes for node node_id
   */
  Real volumeSum(dof_id_type node_id);

  /**
   * Function to return summation of volumes of neighbor nodes using in the deformation gradient
   * calculation for bond connecting node node_id and its neighbor neighbor_id
   */
  Real dgBondVolumeSum(dof_id_type node_id, unsigned int neighbor_id);

  /**
   * Function to return summation of volumes of dgBondVolumeSum at node node_id
   */
  Real dgNodeVolumeSum(dof_id_type node_id);

  /**
   * Function to return number of neighbor for node node_id
   */
  unsigned int nneighbors(dof_id_type node_id);

  /**
   * Function to return mesh_spacing
   */
  Real mesh_spacing(dof_id_type node_id);

  /**
   * Function to return horizon size
   */
  Real horizon(dof_id_type node_id);

  /**
   * Function to check whether a material point falls within a given crack gometry
   */
  bool checkInside(Point start, Point end, Point point, Real width, Real tol = 0);

  /**
   * Function to check whether a bond crosses crack surface
   */
  bool checkCrackIntersection(Point A, Point B, Point C, Point D, Real width);

  /**
   * Function to check whether a segment crosses another segment
   */
  bool checkSegmentIntersection(Point A, Point B, Point C, Point D);

protected:
  ///@{ Horizon size
  const Real _horizon_radius;
  const Real _horizon_number;
  ///@}

  ///@{ Information for crack generation
  const bool _has_cracks;
  std::vector<Point> _cracks_start;
  std::vector<Point> _cracks_end;
  std::vector<Real> _cracks_width;
  ///@}

  /// Mesh dimension
  unsigned int _dim;

  /// List of material points with their data information
  std::vector<PD_Node> _pdnode;

  /// Number of total material points
  unsigned int _total_nodes;

  /// Number of total bonds
  unsigned int _total_bonds;

  /// Neighbor list for each material point
  std::vector<std::vector<dof_id_type>> _node_neighbors;

  /// N nearest neighbor list for each material point
  std::vector<std::vector<dof_id_type>> _node_n_nearest_neighbors;

  /// List of bonds associated with each material point
  std::vector<std::vector<dof_id_type>> _node_bonds;

  /// Neighbor list for bond-associated horizon
  std::vector<std::vector<std::vector<unsigned int>>> _dg_nodeinfo;

  /// Total volume of each bond used in bond-associated deformation gradient
  std::vector<std::vector<Real>> _dg_bond_volumesum;

  /// Total volume of all bonds used in bond-associated deformation gradient
  std::vector<Real> _dg_node_volumesum;
};

#endif // MESHBASEPD_H
