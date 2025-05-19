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

struct NeighborInfo
{
  std::vector<std::vector<Real>> solution_values;
  std::vector<Real> distances;
};

namespace ICStrategyForNewlyActivated
{
enum Type
{
  IC_DEFAULT,
  IC_EXTRAPOLATE_FIRST_LAYER,
  IC_EXTRAPOLATE_SECOND_LAYER
};
}

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

  inline void MPIAllgatherVectorAll_Int(const std::vector<int> & local_vec,
                                        std::vector<int> & global_vec,
                                        MPI_Comm comm = MPI_COMM_WORLD) const
  {
    int nProc, myRank;
    MPI_Comm_size(comm, &nProc);
    MPI_Comm_rank(comm, &myRank);

    int local_size = static_cast<int>(local_vec.size());
    std::vector<int> recv_counts(nProc);
    MPI_Allgather(&local_size, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, comm);

    std::vector<int> displs(nProc, 0);
    std::partial_sum(recv_counts.begin(), recv_counts.end() - 1, displs.begin() + 1);

    int total_size = std::accumulate(recv_counts.begin(), recv_counts.end(), 0);
    global_vec.resize(total_size);

    MPI_Allgatherv(local_vec.data(),
                   local_size,
                   MPI_INT,
                   global_vec.data(),
                   recv_counts.data(),
                   displs.data(),
                   MPI_INT,
                   comm);
  }

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

  std::map<dof_id_type, NeighborInfo> _newlyactivated_node_to_second_neighbors;

  ///
  std::unordered_set<dof_id_type> _newactivated_nodes;
  std::unordered_set<dof_id_type> _newactivated_nodes_first_pass;

  std::string _ic_strategy_string;
  ICStrategyForNewlyActivated::Type _ic_strategy;
  int _inactive_subdomain_ID /*this is actually inactive element*/;

  /// @brief find the second layer of neighbors for each element
  /// @param sys
  /// @param displaced
  void computeSecondNeighborInfo(SystemBase & sys, bool displaced);
  void verifySecondNeighborInfo();
  bool nodeIsNewlyActivated(dof_id_type node_id) const;
  void gatherGlobalNewlyActivatedNodes(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);
  void findNewlyActivatedNodes(
      const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems);

  inline ICStrategyForNewlyActivated::Type parseString2ICStrategy(const std::string & input)
  {
    if (input == "IC_EXTRAPOLATE_FIRST_LAYER")
      return ICStrategyForNewlyActivated::IC_EXTRAPOLATE_FIRST_LAYER;
    else if (input == "IC_EXTRAPOLATE_SECOND_LAYER")
      return ICStrategyForNewlyActivated::IC_EXTRAPOLATE_SECOND_LAYER;
    else if (input == "default_value")
      return ICStrategyForNewlyActivated::IC_DEFAULT;
    else
      throw std::invalid_argument("Invalid string for ICStrategyForNewlyActivated: " + input);
  }
  void setCurrentSolutionsOnNewlyActivatedNodes(SystemBase & sys);

  std::vector<dof_id_type> _global_reinitialized_elems;
  std::vector<dof_id_type> _global_newactivated_nodes;
  std::vector<dof_id_type> _global_newactivated_nodes_temp;
  std::vector<dof_id_type> _global_newactivated_nodes_diff;

  void synchronizeReinitializedElems();
  void synchronizeNewActivatedNodes();
  void collectNewActivatedNodesToMaster();
  void requestMasterActivatedNodes();
  void clearGlobalActivatedNodesAtMaster();
  void synchronizeNewActivatedNodes2TempGlobal();
  void computeSetDifference();
  void findMissingNewlyActivatedNodes();
};
