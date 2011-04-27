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
#include "Moose.h"
#include "ParallelUniqueId.h"

class PenetrationThread
{
public:
  PenetrationThread(const MeshBase & mesh,
                    unsigned int master_boundary,
                    unsigned int slave_boundary,
                    std::map<unsigned int, PenetrationLocator::PenetrationInfo *> & penetration_info,
                    bool update_location,
                    std::vector<FEBase * > & fes,
                    FEType & fe_type,
                    NearestNodeLocator & nearest_node,
                    std::vector<std::vector<unsigned int> > & node_to_elem_map,
                    std::vector< unsigned int > & elem_list,
                    std::vector< unsigned short int > & side_list,
                    std::vector< short int > & id_list);
  
  // Splitting Constructor
  PenetrationThread(PenetrationThread & x, Threads::split split);

  void operator() (const NodeIdRange & range);

  void join(const PenetrationThread & other);

protected:
  // The Mesh
  const MeshBase & _mesh;
  unsigned int _master_boundary;
  unsigned int _slave_boundary;

  // This is the info map we're actually filling here
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *> & _penetration_info;

  bool _update_location;

  std::vector<FEBase * > & _fes;

  FEType & _fe_type;

  NearestNodeLocator & _nearest_node;

  std::vector<std::vector<unsigned int> > & _node_to_elem_map;

  std::vector< unsigned int > & _elem_list;
  std::vector< unsigned short int > & _side_list;
  std::vector< short int > & _id_list;

  unsigned int _n_elems;

  THREAD_ID _tid;
};

#endif //PENETRATIONTHREAD_H
