//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "NodalPatchRecoveryBase.h"
#include "KDTree.h"

/**
 * @note Throughout this file, the term "active" refer to elements that are part of the
 * computational domain where PDEs are being solved. This term shall not be confused with "active"
 * in the context of adaptive mesh refinement.
 */

/**
 * Strategies for (re)initializing the solution:
 *
 * - IC: Use the initial condition for the variable
 * - POLYNOMIAL_NEIGHBOR: Fit a polynomial using the neighbor active elements
 * - POLYNOMIAL_WHOLE: Fit a polynomial using all active elements
 * - POLYNOMIAL_NEARBY: Fit a polynomial using nearby active elements whose distance is below a
 *                      specified threshold.
 */
enum class ReinitStrategy
{
  IC,
  POLYNOMIAL_NEIGHBOR,
  POLYNOMIAL_WHOLE,
  POLYNOMIAL_NEARBY
};

/**
 * Base class for mesh modifiers modifying element subdomains
 */
class ElementSubdomainModifierBase : public ElementUserObject
{
public:
  static InputParameters validParams();

  ElementSubdomainModifierBase(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void meshChanged() override;

protected:
  /**
   * Modify the element subdomains
   *
   * This method performs the following mesh/system modifications given the list of element
   * subdomain assignment:
   *   1. Moving boundaries are created (if it doesn't already exist) according to the specified
   *      subdomain pairs.
   *   2. Active (in the context of adaptivity) elements' subdomain IDs are modified per request.
   *   3. Existing moving boundaries are updated according to the new subdomains.
   *   4. The equation system is reinitialized to (de)allocate for the new subdomains.
   *   5. Variables and stateful material properties on elements which have moved into specified
          subdomains are reinitialized.
   *
   * @param moved_elems A dictionary of element subdomain assignments. Key of this dictionary is the
   * element ID, first entry of the value pair is the subdomain ID this element is moving _from_,
   * and the second entry of the value pair is the subdomain ID this element is moving _to_.
   */
  virtual void
  modify(const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  /// Determine if a subdomain is to be reinitialized
  bool subdomainIsReinitialized(SubdomainID id) const;

  /// Pointer to the displaced problem
  DisplacedProblem * _displaced_problem;

  /// Displaced mesh
  MooseMesh * _displaced_mesh;

  /// Boundary names associated with each moving boundary ID
  std::unordered_map<BoundaryID, BoundaryName> _moving_boundary_names;

  /// Nonlinear system
  NonlinearSystemBase & _nl_sys;

  /// Auxiliary system
  AuxiliarySystem & _aux_sys;

private:
  /// Create moving boundaries
  void createMovingBoundaries(MooseMesh & mesh);

  void applySubdomainChanges(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems,
      MooseMesh & mesh);

  /// Change the subdomain ID of all ancestor elements
  void setAncestorsSubdomainIDs(Elem * elem, const SubdomainID subdomain_id);

  void gatherMovingBoundaryChanges(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  void gatherMovingBoundaryChangesHelper(const Elem * elem,
                                         unsigned short side,
                                         const Elem * neigh,
                                         unsigned short neigh_side);

  void applyMovingBoundaryChanges(MooseMesh & mesh);

  /// Update boundaries for adaptive mesh from the parent to children elements
  void updateAMRMovingBoundary(MooseMesh & mesh);

  void findReinitializedElemsAndNodes(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  /// Set old and older solutions to reinitialized elements and nodes
  void setOldAndOlderSolutions(SystemBase & sys,
                               ConstElemRange & elem_range,
                               ConstBndNodeRange & bnd_node_range);

  /// Determine if a node is newly reinitialized
  bool nodeIsNewlyReinitialized(dof_id_type node_id) const;

  /// Reinitialize variables on range of elements and nodes to be reinitialized
  void applyIC(bool displaced);

  /// Reinitialize stateful material properties on range of elements and nodes to be reinitialized
  void initElementStatefulProps(bool displaced);

  /// Range of reinitialized elements
  ConstElemRange & reinitializedElemRange(bool displaced = false);

  /// Range of reinitialized nodes
  ConstNodeRange & reinitializedNodeRange();

  /// Range of reinitialized boundary nodes
  ConstBndNodeRange & reinitializedBndNodeRange(bool displaced = false);

  /// Return the range of nodes (Node*) extracted from reinitialized boundary nodes (BndNode*)
  ConstNodeRange & reinitializedNodeRangeFromBndNodes(bool displaced);

  /// Reinitialize moved elements whose new subdomain is in this list
  std::vector<SubdomainID> _subdomain_ids_to_reinitialize;

  /// Whether to reinitialize moved elements whose old subdomain was in _reinitialize_subdomains
  const bool _old_subdomain_reinitialized;

  /// Moving boundaries associated with each subdomain pair
  typedef std::pair<SubdomainID, SubdomainID> SubdomainPair;
  std::unordered_map<SubdomainPair, BoundaryID> _moving_boundaries;

  /// Element sides to be added
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>>
      _add_element_sides;

  /// Element sides to be removed
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>>
      _remove_element_sides;

  /// Neighbor sides to be added
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>>
      _add_neighbor_sides;

  /// Neighbor sides to be removed
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>>
      _remove_neighbor_sides;

  /// Reinitialized elements
  std::unordered_set<dof_id_type> _reinitialized_elems;
  /// Range of reinitialized elements
  std::unique_ptr<ConstElemRange> _reinitialized_elem_range;
  /// Range of reinitialized elements on the displaced mesh
  std::unique_ptr<ConstElemRange> _reinitialized_displaced_elem_range;

  /// Reinitialized nodes
  std::unordered_set<dof_id_type> _reinitialized_nodes;
  /// Range of reinitialized nodes
  std::unique_ptr<ConstNodeRange> _reinitialized_node_range;
  /// Range of reinitialized boundary nodes
  std::unique_ptr<ConstBndNodeRange> _reinitialized_bnd_node_range;
  /// Range of reinitialized boundary nodes on the displaced mesh
  std::unique_ptr<ConstBndNodeRange> _reinitialized_displaced_bnd_node_range;
  /// Reinitialized boundary nodes in ConstNodeRange format (non-displaced mesh)
  std::unique_ptr<ConstNodeRange> _reinitialized_node_range_from_bnd_nodes;
  /// Reinitialized boundary nodes in ConstNodeRange format (displaced mesh)
  std::unique_ptr<ConstNodeRange> _reinitialized_displaced_node_range_from_bnd_nodes;

  /// Non-reinitialized nodes on reinitialized elements
  std::unordered_set<dof_id_type> _non_reinit_nodes_on_reinit_elems;

  /// A map from variable name to a pair of:
  /// (1) a vector of DOF IDs associated with non-reinitialized nodes on reinitialized elements, and
  /// (2) the corresponding solution values at those DOFs.
  /// This map is used to preserve solution data for variables that should not be reinitialized
  /// even though they reside on reinitialized elements.
  std::map<VariableName, std::pair<std::vector<dof_id_type>, std::vector<Number>>>
      _var_to_dofs_values_from_nonreinit_nodes;

  /// The strategy used to apply IC on newly activated nodes
  std::vector<ReinitStrategy> _reinit_strategy;

  /// @brief Names of the NodalPatchRecoveryBase user objects
  const std::vector<UserObjectName> _npr_names;

  /// @brief Apply initial conditions using polynomial extrapolation
  std::vector<const NodalPatchRecoveryBase *> _npr;

  /// @brief List of variable names to be initialized for IC
  std::vector<VariableName> _reinit_vars;

  /// @brief map from variable name to the index of the nodal patch recovery user object in `_npr`
  std::map<VariableName, unsigned int> _var_name_to_npr_idx;

  /// @brief A map to map reinitialization strategies to their corresponding solved element IDs
  std::map<ReinitStrategy, std::vector<dof_id_type>> _solved_elem_ids_for_npr;

  /// Elements that have been reinitialized due to subdomain changes,
  /// gathered across all processors using MPI
  std::vector<dof_id_type> _global_reinitialized_elems;

  /// POLYNOMIAL_NEARBY related parameters
  /// @brief Minimum number of nearby elements required in the polynomial extrapolation patch.
  int _nearby_element_threshold = 1;

  /// @brief Centroids of all solved elements used for k-d tree construction.
  std::vector<Point> _centroids_of_elements;

  /// @brief Maximum number of elements allowed in a leaf node of the k-d tree.
  int _leaf_max_size = 10;

  /// @brief k-d tree used for neighbor solved element search in polynomial extrapolation.
  KDTree * _kd_tree = nullptr;

  /// @brief Mapping from the k-d tree node index to the corresponding element ID.
  std::vector<dof_id_type> _kd_tree_sequence_elem_id_map;

  /// @brief Minimum diagonal length among the loose bounding boxes of
  /// all solved elements (i.e., elements within the computational domain,
  /// excluding those in _reinitialized_elems).
  /// This value is used to compute the initial search radius for the k-d tree search,
  /// where the radius is estimated as _nearby_element_threshold multiplied by _min_diag_length.
  double _min_diag_length = std::numeric_limits<double>::max();

  /// @brief Radius threshold for the k-d tree neighbor search.
  /// By default, it is initialized as _nearby_element_threshold * _min_diag_length,
  /// but the user can override this value by explicitly setting _nearby_distance_threshold.
  double _nearby_distance_threshold = -1;

  /// Perform a global MPI gather of reinitialized element IDs across all processors.
  /// Results are stored in `_global_reinitialized_elems`.
  void synchronizeReinitializedElems();

  /// @brief Gather neighbor elements for newly activated nodes based on the reinitialization strategy.
  void gatherNeighborElementsForActivatedNodes(ReinitStrategy & reinit_strategy);

  /// @brief Project initial conditions using NodalPatchRecoveryBase user objects
  void projectNprIC(const VariableName & var_name, bool displaced, ReinitStrategy reinit_strategy);

  /// @brief Store values from non-reinitialized nodes on reinitialized elements
  void storeValuesFromNonReinitNodes(const std::set<VariableName> & vars_names);

  /// @brief Restore values to non-reinitialized nodes on reinitialized elements
  void restoreValuesToNonReinitNodes(const std::set<VariableName> & vars_names);
};
