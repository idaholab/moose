/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PENETRATIONTHREAD_H
#define PENETRATIONTHREAD_H

#include "PenetrationLocator.h"
#include "ParallelUniqueId.h"
#include "MooseVariable.h"

class PenetrationThread
{
public:

  PenetrationThread(SubProblem & subproblem,
                    const MooseMesh & mesh,
                    BoundaryID master_boundary,
                    BoundaryID slave_boundary,
                    std::map<unsigned int, PenetrationLocator::PenetrationInfo *> & penetration_info,
                    bool update_location,
                    Real tangential_tolerance,
                    bool do_normal_smoothing,
                    Real normal_smoothing_distance,
                    PenetrationLocator::NORMAL_SMOOTHING_METHOD normal_smoothing_method,
                    std::vector<FEBase * > & fes,
                    FEType & fe_type,
                    NearestNodeLocator & nearest_node,
                    std::map<unsigned int, std::vector<unsigned int> > & node_to_elem_map,
                    std::vector< unsigned int > & elem_list,
                    std::vector< unsigned short int > & side_list,
                    std::vector< short int > & id_list);

  // Splitting Constructor
  PenetrationThread(PenetrationThread & x, Threads::split split);

  void operator() (const NodeIdRange & range);

  void join(const PenetrationThread & other);

protected:
  SubProblem & _subproblem;
  // The Mesh
  const MooseMesh & _mesh;
  BoundaryID _master_boundary;
  BoundaryID _slave_boundary;

  // This is the info map we're actually filling here
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *> & _penetration_info;

  bool _update_location;
  Real _tangential_tolerance;
  bool _do_normal_smoothing;
  Real _normal_smoothing_distance;
  PenetrationLocator::NORMAL_SMOOTHING_METHOD _normal_smoothing_method;
  MooseVariable * _nodal_normal_x;
  MooseVariable * _nodal_normal_y;
  MooseVariable * _nodal_normal_z;

  std::vector<FEBase * > & _fes;

  FEType & _fe_type;

  NearestNodeLocator & _nearest_node;

  std::map<unsigned int, std::vector<unsigned int> > & _node_to_elem_map;

  std::vector< unsigned int > & _elem_list;
  std::vector< unsigned short int > & _side_list;
  std::vector< short int > & _id_list;

  unsigned int _n_elems;

  THREAD_ID _tid;

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

  CompeteInteractionResult
  competeInteractions(PenetrationLocator::PenetrationInfo * pi1,
                      PenetrationLocator::PenetrationInfo * pi2);

  CommonEdgeResult
  interactionsOffCommonEdge(PenetrationLocator::PenetrationInfo * pi1,
                            PenetrationLocator::PenetrationInfo * pi2);

  bool
  findRidgeContactPoint(Point &contact_point,
                        Real & tangential_distance,
                        Node* &closest_node,
                        unsigned int &index,
                        Point &contact_point_ref,
                        std::vector<PenetrationLocator::PenetrationInfo*> &p_info,
                        const unsigned int index1,
                        const unsigned int index2);

  void
  getSideCornerNodes(Elem* side,
                     std::vector<Node*> &corner_nodes);

  bool
  restrictPointToSpecifiedEdgeOfFace(Point& p,
                                     Node* &closest_node,
                                     const Elem* side,
                                     const std::vector<Node*> &edge_nodes);
  bool
  restrictPointToFace(Point& p,
                      Node* &closest_node,
                      const Elem* side);

  bool
  isFaceReasonableCandidate(const Elem * master_elem,
                            const Elem * side,
                            FEBase * fe,
                            const Point & slave_point,
                            const Real tangential_tolerance);

  void
  smoothNormal(PenetrationLocator::PenetrationInfo* info,
               std::vector<PenetrationLocator::PenetrationInfo*> & p_info);


  void
  getSmoothingEdgesAndWeights(const Point& p,
                              const Elem* side,
                              std::vector<std::vector<Node*> > &edge_nodes,
                              std::vector<Real> &edge_face_weights);

  void
  findNeighborsInInfoVector(std::vector<PenetrationLocator::PenetrationInfo*> &p_info,
                            std::vector<std::vector<Node*> > &edge_nodes,
                            std::vector<PenetrationLocator::PenetrationInfo*> &neighbor_info);

  void
  findNeighborsFromNodeElemMap(PenetrationLocator::PenetrationInfo* info,
                               std::vector<std::vector<Node*> > &edge_nodes,
                               std::vector<PenetrationLocator::PenetrationInfo*> &neighbor_info,
                               std::vector<PenetrationLocator::PenetrationInfo*> & p_info);

  void
  computeSlip( FEBase & fe,
               PenetrationLocator::PenetrationInfo & info );

  void
  switchInfo( PenetrationLocator::PenetrationInfo * & info,
              PenetrationLocator::PenetrationInfo * & infoNew );

  struct RidgeData
  {
    unsigned int _index;
    Point _closest_coor;
    Point _closest_coor_ref;
    Real _tangential_distance;
    Node* _closest_node;
  };

  struct RidgeSetData
  {
    Real _distance;
    Point _closest_coor;
    Node* _closest_node;
    std::vector<RidgeData> _ridge_data_vec;
  };
};

#endif //PENETRATIONTHREAD_H
