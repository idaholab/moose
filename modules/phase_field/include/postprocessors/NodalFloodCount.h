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

#include "periodic_boundaries.h"
//Forward Declarations
class NodalFloodCount;
class MooseMesh;
class MooseVariable;

template<>
InputParameters validParams<NodalFloodCount>();

/**
 * This object will mark nodes of continous regions all with a unique number for the purpose of
 * counting or "coloring" unique regions in a solution.  It is designed to work with either a
 * single variable, or multiple variables.
 *
 * Note:  When inspecting multiple variables, those variables must not have regions of interest
 *        that overlap or they will not be correctly colored.
 */
class NodalFloodCount : public ElementPostprocessor
{
public:
  NodalFloodCount(const std::string & name, InputParameters parameters);
  virtual ~NodalFloodCount() {}

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();
  virtual Real getValue();

  // Get the bubble map
  Real getNodeValue(unsigned int node_id) const { return _bubble_map.at(node_id); }

protected:
  class BubbleData
  {
  public:
    BubbleData(std::set<unsigned int> & nodes, unsigned int var_idx) :
        _nodes(nodes),
        _var_idx(var_idx)
      {}

    std::set<unsigned int> _nodes;
    unsigned int _var_idx;
  };

  /**
   * This method will "mark" all nodes on neighboring elements that
   * are above the supplied threshold
   */
  void flood(const Node *node, unsigned int region, int current_idx);

  /**
   * These routines packs/unpack the _bubble_map data into a structure suitable for parallel
   * communication operations. See the comments in these routines for the exact
   * data structure layout.
   */
  void pack(std::vector<unsigned int> &) const;
  void unpack(const std::vector<unsigned int> &);

  /**
   * This routine merges the data in _bubble_sets from separate threads/processes to resolve
   * any bubbles that were counted as unique by multiple processors
   */
  void mergeSets();

  /**
   * This routine returns whether or not a nodal value is valid on a particular processor
   * (i.e. includes values owned by this processor and ghost nodes)
   */
  bool isNodeValueValid(unsigned int node_id) const;

  /**
   * This routine adds the periodic node information to our data structure prior to packing the data
   * this makes those periodic neighbors appear much like ghosted nodes in a multiprocessor setting
   */
  unsigned int appendPeriodicNeighborNodes(std::vector<std::set<unsigned int> > & data) const;

  /// The threshold above which neighboring nodes are flooding with adjacent markings
  Real _threshold;

  /// A reference to the mesh
  MooseMesh & _mesh;

  /**
   * This variable is used to build the periodic node map.
   * Assumption: We are going to assume that either all variables are periodic or none are.
   *             This assumption can be relaxed at a later time if necessary.
   */
  unsigned int _var_number;

  /// The data structure used to hold the map of marked nodes.
  std::map<unsigned int, int> _bubble_map;

  /// The data structure used to marshall the data between processes and/or threads
  std::vector<unsigned int> _packed_data;

  /// The data structure used to find neighboring elements give a node ID
  std::vector< std::vector< const Elem * > > _nodes_to_elem_map;

  /// This data structure is used to keep track of which bubbles are owned by which variables (index).
  std::vector<unsigned int> _region_to_var_idx;

  /// The data structure used to join partial bubbles between prcoesses and/or threads
  std::list<BubbleData> _bubble_sets;

  /// The scalar counter used during the marking stage of the flood algorithm
  unsigned int _region_count;

  /// A pointer to the periodic boundary constraints object
  PeriodicBoundaries *_pbs;

  /**
   * The data structure which is a list of nodes that are constrained to other nodes
   * based on the imposed periodic boundary conditions.
   */
  std::multimap<unsigned int, unsigned int> _periodic_node_map;
};

#endif
