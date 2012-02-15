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

#ifndef NODALFLOODCOUNT_H
#define NODALFLOODCOUNT_H

#include "ElementPostprocessor.h"

#include <list>
#include <vector>
#include <set>

//Forward Declarations
class NodalFloodCount;
class MooseMesh;
class MooseVariable;
class libMesh::DofMap;

template<>
InputParameters validParams<NodalFloodCount>();

class NodalFloodCount : public ElementPostprocessor
{
public:
  NodalFloodCount(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const Postprocessor & y);
  virtual Real getValue();

protected:
  /**
   * This method will "mark" all nodes on neighboring elements that
   * are above the supplied threshold
   */
  void flood(const Node *node, unsigned int region);

  /**
   * These routines packs/unpack the _bubble_map data into a structure suitable for parallel
   * communication operations.
   */
  void pack(std::vector<unsigned int> &) const;
  void unpack(const std::vector<unsigned int> &);

  /**
   * This routine merges the data in _bubble_sets to resolve any
   * bubbles that were counted as unique by multiple processors
   */
  void mergeSets();

  Real _threshold;
  MooseMesh & _mesh;
  MooseVariable & _moose_var;
  unsigned int _var_number;

  std::map<unsigned int, int> _bubble_map;
  std::vector<unsigned int> _packed_data;
  std::vector< std::vector< const Elem * > > _nodes_to_elem_map;
  std::list<std::set<unsigned int> > _bubble_sets;
//  const NumericVector<Number> &_serialized_solution;

  unsigned int _region_count;
  
  //DEBUG
//  libMesh::DofMap & _dof_map;
};

#endif
