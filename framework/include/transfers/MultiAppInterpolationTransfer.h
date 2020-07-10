//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MultiAppConservativeTransfer.h"
#include "MooseVariableFieldBase.h"

#include "libmesh/mesh_base.h"

// Forward declarations
class MultiAppInterpolationTransfer;

template <>
InputParameters validParams<MultiAppInterpolationTransfer>();

namespace libMesh
{
template <unsigned int>
class InverseDistanceInterpolation;
}

/**
 * Copy the value to the target domain from the nearest node in the source domain.
 */
class MultiAppInterpolationTransfer : public MultiAppConservativeTransfer
{
public:
  static InputParameters validParams();

  MultiAppInterpolationTransfer(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * Return the nearest node to the point p.
   * @param p The point you want to find the nearest node to.
   * @param distance This will hold the distance between the returned node and p
   * @param nodes_begin - iterator to the beginning of the node list
   * @param nodes_end - iterator to the end of the node list
   * @return The Node closest to point p.
   */
  Node * getNearestNode(const Point & p,
                        Real & distance,
                        const MeshBase::const_node_iterator & nodes_begin,
                        const MeshBase::const_node_iterator & nodes_end);

  void
  fillSourceInterpolationPoints(FEProblemBase & from_problem,
                                const MooseVariableFieldBase & from_var,
                                const Point & from_app_position,
                                std::unique_ptr<InverseDistanceInterpolation<LIBMESH_DIM>> & idi);

  void
  interpolateTargetPoints(FEProblemBase & to_problem,
                          MooseVariableFieldBase & to_var,
                          NumericVector<Real> & to_solution,
                          const Point & to_app_position,
                          const std::unique_ptr<InverseDistanceInterpolation<LIBMESH_DIM>> & idi);

  void
  subdomainIDsNode(MooseMesh & mesh, const Node & node, std::set<subdomain_id_type> & subdomainids);

  void computeTransformation(const MooseMesh & mesh,
                             std::unordered_map<dof_id_type, Point> & transformation);

  unsigned int _num_points;
  Real _power;
  MooseEnum _interp_type;
  Real _radius;
  // How much we want to shrink gap
  Real _shrink_gap_width;
  // Which mesh we want to shrink
  MooseEnum _shrink_mesh;
  // Which gap blocks want to exclude during solution transfers
  std::vector<SubdomainName> _exclude_gap_blocks;
  // How small we can consider two points are identical
  Real _distance_tol;
};
