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

class PenetrationThread
{
public:
  PenetrationThread(const MooseMesh & mesh,
                    BoundaryID master_boundary,
                    BoundaryID slave_boundary,
                    std::map<unsigned int, PenetrationLocator::PenetrationInfo *> & penetration_info,
                    bool update_location,
                    Real tangential_tolerance,
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
  // The Mesh
  const MooseMesh & _mesh;
  BoundaryID _master_boundary;
  BoundaryID _slave_boundary;

  // This is the info map we're actually filling here
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *> & _penetration_info;

  bool _update_location;
  Real _tangential_tolerance;

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
                       PenetrationLocator::PenetrationInfo * pi1,
                       PenetrationLocator::PenetrationInfo * pi2,
                       Real & tangential_distance,
                       Node* &closest_node);

  void
  getSideCornerNodes(Elem* side,
                     std::vector<Node*> &corner_nodes);

  bool
  restrictPointToSpecifiedEdgeOfFace(Point& p,
                                     Node* &closest_node,
                                     const Elem* side,
                                     const std::vector<Node*> &edge_nodes);
  struct RidgeData
  {
    unsigned int _index1;
    unsigned int _index2;
    Point _closest_coor;
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
