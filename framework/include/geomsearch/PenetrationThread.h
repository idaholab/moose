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
#include "MooseTypes.h"
#include "PenetrationLocator.h"

#include "libmesh/elem_side_builder.h"

// Forward declarations
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<VectorValue<Real>> VectorMooseVariable;

class PenetrationThread
{
public:
  PenetrationThread(
      SubProblem & subproblem,
      const MooseMesh & mesh,
      BoundaryID primary_boundary,
      BoundaryID secondary_boundary,
      std::map<dof_id_type, PenetrationInfo *> & penetration_info,
      bool check_whether_reasonable,
      bool update_location,
      Real tangential_tolerance,
      bool do_normal_smoothing,
      Real normal_smoothing_distance,
      PenetrationLocator::NORMAL_SMOOTHING_METHOD normal_smoothing_method,
      std::vector<std::vector<FEBase *>> & fes,
      FEType & fe_type,
      NearestNodeLocator & nearest_node,
      const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map,
      const std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> & bc_tuples);

  // Splitting Constructor
  PenetrationThread(PenetrationThread & x, Threads::split split);

  void operator()(const NodeIdRange & range);

  void join(const PenetrationThread & other);

  /// List of secondary nodes for which penetration was not detected in the current patch and for which patch has to be updated.
  std::vector<dof_id_type> _recheck_secondary_nodes;

protected:
  SubProblem & _subproblem;
  // The Mesh
  const MooseMesh & _mesh;
  BoundaryID _primary_boundary;
  BoundaryID _secondary_boundary;

  // This is the info map we're actually filling here
  std::map<dof_id_type, PenetrationInfo *> & _penetration_info;

  bool _check_whether_reasonable;
  bool _update_location;
  Real _tangential_tolerance;
  bool _do_normal_smoothing;
  Real _normal_smoothing_distance;
  PenetrationLocator::NORMAL_SMOOTHING_METHOD _normal_smoothing_method;
  MooseVariable * _nodal_normal_x;
  MooseVariable * _nodal_normal_y;
  MooseVariable * _nodal_normal_z;

  std::vector<std::vector<FEBase *>> & _fes;

  FEType & _fe_type;

  NearestNodeLocator & _nearest_node;

  const std::map<dof_id_type, std::vector<dof_id_type>> & _node_to_elem_map;

  // Each boundary condition tuple has three entries, (0=elem-id, 1=side-id, 2=bc-id)
  const std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> & _bc_tuples;

  THREAD_ID _tid;

  /// Helper for building element sides without extraneous allocation
  ElemSideBuilder _elem_side_builder;

  enum CompeteInteractionResult
  {
    FIRST_WINS,
    SECOND_WINS,
    NEITHER_WINS
  };

  enum CommonEdgeResult
  {
    NO_COMMON,
    COMMON_EDGE,
    COMMON_NODE,
    EDGE_AND_COMMON_NODE
  };

  /**
   * When interactions are identified between a node and two faces, compete between the faces
   * to determine whether first (pi1) or second (pi2) interaction is stronger
   * @param pi1 Pointer to the PenetrationInfo object for the first face
   * @param pi2 Pointer to the PenetrationInfo object for the second face
   * @return Appropriate ComputeInterationResult enum entry identifying which face is a better match
   */
  CompeteInteractionResult competeInteractions(PenetrationInfo * pi1, PenetrationInfo * pi2);

  /**
   * Determine whether first (pi1) or second (pi2) interaction is stronger when it is known
   * that the node projects to both of the two competing faces
   * @param pi1 Pointer to the PenetrationInfo object for the first face
   * @param pi2 Pointer to the PenetrationInfo object for the second face
   * @return Appropriate ComputeInterationResult enum entry identifying which face is a better match
   */
  CompeteInteractionResult competeInteractionsBothOnFace(PenetrationInfo * pi1,
                                                         PenetrationInfo * pi2);

  CommonEdgeResult interactionsOffCommonEdge(PenetrationInfo * pi1, PenetrationInfo * pi2);

  bool findRidgeContactPoint(Point & contact_point,
                             Real & tangential_distance,
                             const Node *& closest_node,
                             unsigned int & index,
                             Point & contact_point_ref,
                             std::vector<PenetrationInfo *> & p_info,
                             const unsigned int index1,
                             const unsigned int index2);

  void getSideCornerNodes(const Elem * side, std::vector<const Node *> & corner_nodes);

  bool restrictPointToSpecifiedEdgeOfFace(Point & p,
                                          const Node *& closest_node,
                                          const Elem * side,
                                          const std::vector<const Node *> & edge_nodes);
  bool restrictPointToFace(Point & p, const Node *& closest_node, const Elem * side);

  bool isFaceReasonableCandidate(const Elem * primary_elem,
                                 const Elem * side,
                                 FEBase * fe,
                                 const Point * secondary_point,
                                 const Real tangential_tolerance);

  void smoothNormal(PenetrationInfo * info, std::vector<PenetrationInfo *> & p_info);

  void getSmoothingFacesAndWeights(PenetrationInfo * info,
                                   std::vector<PenetrationInfo *> & edge_face_info,
                                   std::vector<Real> & edge_face_weights,
                                   std::vector<PenetrationInfo *> & p_info);
  void getSmoothingEdgeNodesAndWeights(const Point & p,
                                       const Elem * side,
                                       std::vector<std::vector<const Node *>> & edge_nodes,
                                       std::vector<Real> & edge_face_weights);

  void getInfoForFacesWithCommonNodes(const Node * secondary_node,
                                      const std::set<dof_id_type> & elems_to_exclude,
                                      const std::vector<const Node *> edge_nodes,
                                      std::vector<PenetrationInfo *> & face_info_comm_edge,
                                      std::vector<PenetrationInfo *> & p_info);

  void getInfoForElem(std::vector<PenetrationInfo *> & thisElemInfo,
                      std::vector<PenetrationInfo *> & p_info,
                      const Elem * elem);

  void createInfoForElem(std::vector<PenetrationInfo *> & thisElemInfo,
                         std::vector<PenetrationInfo *> & p_info,
                         const Node * secondary_node,
                         const Elem * elem,
                         const std::vector<const Node *> & nodes_that_must_be_on_side,
                         const bool check_whether_reasonable = false);

  void getSidesOnPrimaryBoundary(std::vector<unsigned int> & sides, const Elem * const elem);

  void computeSlip(FEBase & fe, PenetrationInfo & info);

  void switchInfo(PenetrationInfo *& info, PenetrationInfo *& infoNew);

  struct RidgeData
  {
    unsigned int _index;
    Point _closest_coor;
    Point _closest_coor_ref;
    Real _tangential_distance;
    const Node * _closest_node;
  };

  struct RidgeSetData
  {
    Real _distance;
    Point _closest_coor;
    const Node * _closest_node;
    std::vector<RidgeData> _ridge_data_vec;
  };
};
