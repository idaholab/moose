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

  void initialSetup() override;
  void meshChanged() override;

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

  void prepareVariableForReinitialization(const VariableName & var_name,
                                          ReinitStrategy reinit_strategy);

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
  void applyIC();

  /// Reinitialize stateful material properties on range of elements and nodes to be reinitialized
  void initElementStatefulProps();

  /// Range of reinitialized elements
  ConstElemRange & reinitializedElemRange();

  /// Range of reinitialized nodes
  ConstNodeRange & reinitializedNodeRange();

  /// Range of reinitialized boundary nodes
  ConstBndNodeRange & reinitializedBndNodeRange();

  /// @brief Gather patch elements for reinitialized elements based onthe reinitialization strategy.
  void gatherPatchElements(const VariableName & var_name, ReinitStrategy reinit_strategy);

  /// @brief Extrapolate polynomial for the given variable onto the reinitialized elements.
  void extrapolatePolynomial(const VariableName & var_name);

  /// @brief Store values from non-reinitialized nodes on reinitialized elements
  void storeOverriddenDofValues(const VariableName & var_name);

  /// @brief Restore values to non-reinitialized nodes on reinitialized elements
  void restoreOverriddenDofValues(const VariableName & var_name);

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

  /// Reinitialized nodes
  std::unordered_set<dof_id_type> _reinitialized_nodes;
  /// Range of reinitialized nodes
  std::unique_ptr<ConstNodeRange> _reinitialized_node_range;
  /// Range of reinitialized boundary nodes
  std::unique_ptr<ConstBndNodeRange> _reinitialized_bnd_node_range;

  /// A map from variable name to a pair of:
  /// (1) a vector of DOF IDs associated with non-reinitialized nodes on reinitialized elements, and
  /// (2) the corresponding solution values at those DOFs.
  /// This map is used to preserve solution data for variables that should not be reinitialized
  /// even though they reside on reinitialized elements.
  std::map<VariableName, std::pair<std::vector<dof_id_type>, std::vector<Number>>>
      _overridden_values_on_reinit_elems;

  /// The strategy used to apply IC on newly activated nodes
  std::vector<ReinitStrategy> _reinit_strategy;

  /// @brief Names of the NodalPatchRecoveryBase user objects
  const std::vector<UserObjectName> _pr_names;

  /// @brief Apply initial conditions using polynomial extrapolation
  std::vector<const NodalPatchRecoveryBase *> _pr;

  /// @brief List of variable names to be initialized for IC
  std::vector<VariableName> _reinit_vars;

  /// @brief map from variable name to the index of the nodal patch recovery user object in `_pr`
  std::map<VariableName, unsigned int> _var_name_to_pr_idx;

  /**
   * @brief local evaluable elements before reinitializing the equation systems
   * Key of the map is the system number, value of the map is a pair of:
   *  (1) an unordered set of evaluable elements for the system
   *  (2) a vector of evaluable element IDs associated with those elements
   */
  std::map<unsigned int, std::pair<std::unordered_set<const Elem *>, std::vector<dof_id_type>>>
      _evaluable_elems;

  /// @brief A map to map reinitialization strategies to their corresponding patch element IDs
  std::map<VariableName, std::vector<dof_id_type>> _patch_elem_ids;

  /// KD-tree related members
  ///@{
  /// @brief Maximum number of elements allowed in a leaf node of the k-d tree.
  int _leaf_max_size = 10;
  /// @brief Centroids of all solved elements used for k-d tree construction.
  std::vector<Point> _kd_tree_points;
  /// @brief Radius threshold for the k-d tree neighbor search.
  double _nearby_distance_threshold;
  ///@}

  /// @brief List of variable names for which overridden DOF values should be restored.
  std::vector<VariableName> _vars_to_restore_overridden_dofs;

  /// @brief Set of processor IDs that have reinitialized elements and nodes.
  std::set<processor_id_type> _global_proc_ids_for_reinit;

private:
  /// Construct a KD-tree from the given elements
  std::unique_ptr<KDTree> constructKDTreeFromElements(const std::vector<dof_id_type> & elems);
};
