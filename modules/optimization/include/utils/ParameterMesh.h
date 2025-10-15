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
#include <functional>

namespace libMesh
{
class System;
class DofMap;
}

using libMesh::RealGradient;

/**
 * Utility class to use an Exodus mesh to define controllable parameters for optimization problems
 * This class will:
 *  - Ensure that controllable parameters defined by the mesh are correctly ordered on optimiation
 * main app and forward or adjoint sub-apps
 *  - Define the parameter space (nodal/elemental and shape function)
 *  - Interpolate parameter values to the forward problem mesh using nodal/elemental shape function
 */
class ParameterMesh
{
public:
  ParameterMesh(const libMesh::FEType & param_type,
                const std::string & exodus_mesh,
                const bool find_closest = false,
                const unsigned int kdtree_candidates = 5);

  /// Enumerations for regularization computations
  enum class RegularizationType
  {
    L2_GRADIENT,
    // Future regularization types can be added here:
    // L1,
    // H1,
    // TV (Total Variation)
  };

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
   * Computes regularization objective value for a given regularization type
   * @param parameter_values  vector of parameter values to compute regularization for
   * @param reg_type  type of regularization (L2_GRADIENT, etc.)
   * @return scalar objective value
   */
  Real computeRegularizationObjective(const std::vector<Real> & parameter_values,
                                      RegularizationType reg_type) const;

  /**
   * Computes regularization gradient for a given regularization type
   * @param parameter_values  vector of parameter values to compute gradient for
   * @param reg_type  type of regularization (L2_GRADIENT, etc.)
   * @return vector of gradient values (same size as parameter_values)
   */
  std::vector<Real> computeRegularizationGradient(const std::vector<Real> & parameter_values,
                                                  RegularizationType reg_type) const;

private:
  /**
   * Template method containing the element loop for regularization computations
   * @param parameter_values  vector of parameter values
   * @param reg_type  type of regularization
   * @param compute_func  lambda function to perform computation at each quadrature point
   * @return result of type T (Real for objective, std::vector<Real> for gradient)
   */
  template <typename T>
  T
  computeRegularizationLoop(const std::vector<Real> & parameter_values,
                            RegularizationType reg_type,
                            const std::function<void(const std::vector<std::vector<Real>> &,
                                                     const std::vector<std::vector<RealGradient>> &,
                                                     const unsigned int,
                                                     const std::vector<dof_id_type> &,
                                                     const std::vector<Real> &,
                                                     T &)> & compute_func) const;

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

  /**
   * Compute regularization objective for a single quadrature point
   * This is the main function users should modify to add new regularization types for objectives
   * @param parameter_values  all parameter values
   * @param phi  shape function values (full array)
   * @param dphi  shape function gradients (full array)
   * @param qp  quadrature point index
   * @param dof_indices  element DOF indices
   * @param JxW  quadrature weights array
   * @param reg_type  type of regularization to compute
   * @return contribution to objective function
   */
  Real computeRegularizationQp(const std::vector<Real> & parameter_values,
                               const std::vector<std::vector<Real>> & phi,
                               const std::vector<std::vector<RealGradient>> & dphi,
                               const unsigned int qp,
                               const std::vector<dof_id_type> & dof_indices,
                               const std::vector<Real> & JxW,
                               RegularizationType reg_type) const;

  /**
   * Compute regularization gradient for a single quadrature point
   * This is the main function users should modify to add new regularization types for gradients
   * @param parameter_values  all parameter values
   * @param phi  shape function values (full array)
   * @param dphi  shape function gradients (full array)
   * @param qp  quadrature point index
   * @param dof_indices  element DOF indices
   * @param JxW  quadrature weights array
   * @param reg_type  type of regularization to compute
   * @param gradient  gradient vector to update
   */
  void computeRegularizationGradientQp(const std::vector<Real> & parameter_values,
                                       const std::vector<std::vector<Real>> & phi,
                                       const std::vector<std::vector<RealGradient>> & dphi,
                                       const unsigned int qp,
                                       const std::vector<dof_id_type> & dof_indices,
                                       const std::vector<Real> & JxW,
                                       RegularizationType reg_type,
                                       std::vector<Real> & gradient) const;
  // Cached values for gradient computations
  const unsigned short int _param_var_id;
  const libMesh::DofMap * _dof_map;
  const libMesh::FEType _fe_type;
};
