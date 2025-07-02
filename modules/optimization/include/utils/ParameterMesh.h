//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "KDTree.h"

#include "libmesh/id_types.h"
#include <unordered_map>
#include "libmesh/parallel.h"
#include "libmesh/fe_type.h"
#include "libmesh/point.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/equation_systems.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/exodusII_io.h"
#include <vector>

namespace libMesh
{
class System;
}

using libMesh::RealGradient;

/**
 * Utility class to use an Exodus mesh to define controllable parameters for optimization problems
 * This class will:
 *  - Ensure that controllable parameters defined by the mesh are correctly ordered on optimiation
 * main app and forward or adjoint sub-apps
 *  - Read initial conditions and bounds from an exodus file
 *  - Define the parameter space (nodal/elemental and shape function)
 *  - Interpolate parameter values to the forward problem mesh using nodal/elemental shape function
 */
class ParameterMesh
{
public:
  ParameterMesh(const libMesh::FEType & param_type,
                const std::string & exodus_mesh,
                const std::vector<std::string> & var_names = {},
                const bool find_closest = false,
                const unsigned int kdtree_candidates = 5);

  /**
   * @return the number of parameters read from the mesh for a single timestep
   */
  dof_id_type size() const { return _param_dofs; }
  /**
   * Interpolate parameters onto the computational mesh
   * getIndexAndWeight is only used by ParameterMeshFunction
   * @param pt location to compute elemnent dof_indices weights
   * @param dof_indices return dof indices for element containing pt
   * @param weights returns element shape function weights at pt
   */
  void getIndexAndWeight(const Point & pt,
                         std::vector<dof_id_type> & dof_indices,
                         std::vector<Real> & weights) const;
  /**
   * Performs inner products of parameters with functions on the computational mesh
   * getIndexAndWeight is only used by ParameterMeshFunction
   * @param pt location to compute elemnent dof_indices weights
   * @param dof_indices return dof indices for element containing pt
   * @param weights returns element shape function gradient weights at pt
   */
  void getIndexAndWeight(const Point & pt,
                         std::vector<dof_id_type> & dof_indices,
                         std::vector<RealGradient> & weights) const;
  /**
   * Initializes parameter data and sets bounds in the main optmiization application
   * getParameterValues is only used by ParameterMeshOptimization
   * @param var_name  variable to read off mesh
   * @param timestep  timestep to read variable off mesh
   * @return vector of variables read off mesh at timestep
   */
  std::vector<Real> getParameterValues(std::string var_name, unsigned int timestep) const;

protected:
  libMesh::Parallel::Communicator _communicator;
  libMesh::ReplicatedMesh _mesh;
  /// Find closest projection points
  const bool _find_closest;
  std::unique_ptr<libMesh::EquationSystems> _eq;
  libMesh::System * _sys;
  std::unique_ptr<libMesh::PointLocatorBase> _point_locator;
  std::unique_ptr<libMesh::ExodusII_IO> _exodusII_io;

  dof_id_type _param_dofs;

  /// Node-based KDTree optimization
  std::vector<Point> _mesh_nodes;
  std::unique_ptr<KDTree> _node_kdtree;
  std::unordered_map<dof_id_type, std::set<const libMesh::Elem *>> _node_to_elements;
  unsigned int _kdtree_candidates;

private:
  /**
   * Returns the point on the parameter mesh that is projected from the test point
   * @param p test point
   * @return Point
   */
  Point projectToMesh(const Point & p) const;

  /**
   * Find closest point on the element to the given point
   * @param elem
   * @param p
   * @return Point
   */
  Point closestPoint(const Elem & elem, const Point & p) const;
};
