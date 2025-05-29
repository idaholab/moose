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

namespace ICStrategyForNewlyActivated
{
enum Type
{
  IC_DEFAULT,
  IC_EXTRAPOLATE_FIRST_LAYER,
  IC_POLYNOMIAL,
  IC_POLYNOMIAL_WHOLE_SOLVED_DOMAIN,
  IC_POLYNOMIAL_THRESHOLD,
};
}

struct NeighborInfo
{
  std::vector<std::vector<Real>> solution_values;
  std::vector<Real> distances;
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

  /// Range of reinitialized boundary nodes
  ConstBndNodeRange & reinitializedBndNodeRange(bool displaced = false);

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
  /// Range of reinitialized boundary nodes
  std::unique_ptr<ConstBndNodeRange> _reinitialized_bnd_node_range;
  /// Range of reinitialized boundary nodes on the displaced mesh
  std::unique_ptr<ConstBndNodeRange> _reinitialized_displaced_bnd_node_range;

  /// @brief Map from newly-activated node ID to a NeighborInfo structure (distances and solutions)
  std::map<dof_id_type, NeighborInfo> _newlyactivated_node_to_first_layer_neighbors_info;

  /// @brief Set of newly activated nodes
  std::unordered_set<dof_id_type> _newactivated_nodes;
  std::unordered_set<dof_id_type> _first_pass_local_activated_nodes;

  std::string _ic_strategy_string;
  ICStrategyForNewlyActivated::Type _ic_strategy;

  /// Inactive subdomain ID
  int _inactive_subdomain_ID;

  /// @brief find the first layer of neighbors for each element
  /// @param sys
  void computeFirstLayerNeighborInfo(SystemBase & sys);

  /**
   * * Check if the node is newly activated.
   * * If all elements (excluding inactive elements) with the node are reinitialized, then the node
   * is newly reinitialized.
   */
  bool nodeIsNewlyActivated(dof_id_type node_id) const;

  /// @brief Collect the complete set of newly activated nodes globally across processors
  /// @param moved_elems Map from element ID to a pair of (old subdomain ID, new subdomain ID)
  /// @details This function gathers all nodes associated with moved elements that are newly
  /// activated, regardless of processor ownership. This is the authoritative global pass
  /// in a parallel algorithm and is used to build the correct global view.
  void identifyGloballyActivatedNodes(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  /// @brief Identify newly activated nodes that are locally owned by the current processor
  /// @param moved_elems Map from element ID to a pair of (old subdomain ID, new subdomain ID)
  /// @details This function processes only the nodes owned by the local processor and stores
  /// the locally owned subset of newly activated nodes. This step is typically performed
  /// after the global gather step to ensure that only owned nodes are used for further
  /// operations such as mesh updates or DoF assignments.
  void identifyLocallyOwnedActivatedNodes(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  /// Elements that have been reinitialized due to subdomain changes,
  /// gathered across all processors using MPI
  std::vector<dof_id_type> _global_reinitialized_elems;

  /// First-pass global collection of all newly activated node IDs from every processor.
  /// This is the authoritative set, as it does not rely on node ownership.
  /// Includes all nodes connected to reinitialized elements, regardless of processor.
  /// Used as a reference for correctness in parallel settings.
  std::vector<dof_id_type> _complete_global_activated_nodes;

  /// Final (second-pass) collection of newly activated node IDs,
  /// filtered to include only those locally owned by this processor.
  /// May miss some nodes in parallel runs due to ownership mismatch.
  /// Used for applying ICs but validated against `_complete_global_activated_nodes`.
  std::vector<dof_id_type> _local_own_gather_global_activated_nodes;

  /// Difference between `_complete_global_activated_nodes` and `_local_own_gather_global_activated_nodes`.
  /// Helps identify missing nodes that should have been activated but were skipped due to
  /// processor ownership constraints. Useful for debugging MPI consistency issues.
  std::vector<dof_id_type> _local_own_gather_global_and_complete_activated_nodes_diff;

  /// Indicates whether each node has had its initial condition (IC) applied.
  /// true = IC already set; false = IC not yet set.
  std::unordered_map<dof_id_type, bool> _node2IC_set;

  /// @brief Names of the NodalPatchRecoveryBase user objects
  const std::vector<UserObjectName> _npr_names;

  /// @brief Apply initial conditions using polynomial extrapolation
  std::vector<const NodalPatchRecoveryBase *> _npr_vec;

  /// @brief List of neighbor elements that share nodes with reinitialized elements
  std::vector<dof_id_type> _solved_elem_ids_for_npr;

  /// IC_POLYNOMIAL_THRESHOLD related parameters
  /// @brief Threshold for checking the closeness of element numbers in polynomial extrapolation
  int _nearby_element_threshold = 1;

  /// @brief centroids of the element
  std::vector<Point> _centroids_elements;

  /// @brief Maximum number of elements in a leaf node of the k-d tree
  int _leaf_max_size = 10;

  /// @brief k-d tree for neighbor element search in polynomial extrapolation
  KDTree * _kd_tree = nullptr;

  /// @brief  Map from k-d tree sequence index to element ID
  std::vector<dof_id_type> _kd_tree_sequence_elem_id_map;

  /// @brief Minimum diagonal length (criteria) for the k-d tree search
  double _min_diag_length = std::numeric_limits<double>::max();

  /// @brief Radius search threshold for k-d tree search
  double _radius_search_threshold = -1;

  /// Using weighted averaging to obtain the solution on the newly-activated nodes
  void setCurrentSolutionsOnNewlyActivatedNodes(SystemBase & sys);

  /// Perform a global MPI gather of reinitialized element IDs across all processors.
  /// Results are stored in `_global_reinitialized_elems`.
  void synchronizeReinitializedElems();

  /// First-pass: Perform a global MPI gather of newly activated node IDs from all processors.
  /// This does **not** check for local ownership and is considered the complete and correct set.
  /// Stores result in `_complete_global_activated_nodes`.
  void gatherCompleteActivatedNodesGlobally();

  /// Second-pass: Gather newly activated nodes that are **locally owned** by this processor only.
  /// This subset may be incomplete in MPI runs, and should be validated against the first pass.
  /// Stores result in `_local_own_gather_global_activated_nodes`.
  void gatherLocalActivatedNodesGlobally();

  /// @brief  An additional check ensures that the number of globally activated nodes
  /// in the second pass is not less than that of the first pass.
  /// If a mismatch is detected, we identify which processor is missing the expected node
  /// using findMissingNewlyActivatedNodes()
  void computeSetDifference();

  /// @brief Identify the processor that is missing the newly activated nodes
  void findMissingNewlyActivatedNodes();

  /// @brief Apply initial conditions for a list of nodes
  void applyICForNodeList(SystemBase & sys, const std::vector<dof_id_type> & nodes);

  /// @brief Apply initial conditions using polynomial nodal patch recovery
  /// @param sys
  void applyIC_Polynomial(SystemBase & sys);

  /// @brief Gather neighbor elements for newly activated nodes
  void gatherNeighborElementsForActivatedNodes();

  inline ICStrategyForNewlyActivated::Type parseString2ICStrategy(const std::string & input)
  {
    if (input == "IC_EXTRAPOLATE_FIRST_LAYER")
      return ICStrategyForNewlyActivated::IC_EXTRAPOLATE_FIRST_LAYER;
    else if (input == "default_value")
      return ICStrategyForNewlyActivated::IC_DEFAULT;
    else if (input == "IC_POLYNOMIAL")
      return ICStrategyForNewlyActivated::IC_POLYNOMIAL;
    else if (input == "IC_POLYNOMIAL_WHOLE_SOLVED_DOMAIN")
      return ICStrategyForNewlyActivated::IC_POLYNOMIAL_WHOLE_SOLVED_DOMAIN;
    else if (input == "IC_POLYNOMIAL_THRESHOLD")
      return ICStrategyForNewlyActivated::IC_POLYNOMIAL_THRESHOLD;
    else
      throw std::invalid_argument("Invalid string for ICStrategyForNewlyActivated: " + input);
  }
};
