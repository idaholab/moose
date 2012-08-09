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

#include "NodalFloodCount.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "NonlinearSystem.h"
#include "FEProblem.h"


//libMesh includes
#include "dof_map.h"
#include "mesh_tools.h"
#include "periodic_boundaries.h"
#include "point_locator_base.h"

#include <algorithm>
#include <limits>

template<>
InputParameters validParams<NodalFloodCount>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addParam<Real>("threshold", 0.5, "The threshold value of the bubble boundary");
  return params;
}

NodalFloodCount::NodalFloodCount(const std::string & name, InputParameters parameters) :
    ElementPostprocessor(name, parameters),
    _threshold(getParam<Real>("threshold")),
    _mesh(_subproblem.mesh()),
    _var_number(_var.number()),
    _region_count(0),
    _pbs(NULL)
{}

void
NodalFloodCount::initialize()
{
  // Get a pointer to the PeriodicBoundaries buried in libMesh
  // TODO: Can we do this in the constructor (i.e. are all objects necessary for this call in existance during ctor?)
  _pbs = dynamic_cast<FEProblem *>(&_subproblem)->getNonlinearSystem().dofMap().get_periodic_boundaries();

  // Clear the bubble marking map
  _bubble_map.clear();

  // Reset the packed data structure
  _packed_data.clear();

  // Reset the region counter
  _region_count = 0;

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh._mesh, _nodes_to_elem_map);

  // TODO: We might only need to build this once if adaptivity is turned off
  _mesh.buildPeriodicNodeMap(_periodic_node_map, _var_number, _pbs);
}

void
NodalFloodCount::execute()
{
  unsigned int n_nodes = _current_elem->n_vertices();
  for (unsigned int i=0; i < n_nodes; ++i)
  {
    const Node *node = _current_elem->get_node(i);

    flood(node, 0);
  }
}

Real
NodalFloodCount::getValue()
{
  // Exchange data in parallel
  pack(_packed_data);
  Parallel::allgather(_packed_data, false);
  unpack(_packed_data);

  mergeSets();

  return _bubble_sets.size();
}

void
NodalFloodCount::threadJoin(const UserObject &y)
{
   const NodalFloodCount & pps = dynamic_cast<const NodalFloodCount &>(y);

   // Pack up the data on both of the threads
   pack(_packed_data);

   std::vector<unsigned int> pps_packed_data;
   pps.pack(pps_packed_data);

   // Append the packed data structures together
   std::copy(pps_packed_data.begin(), pps_packed_data.end(), std::back_inserter(_packed_data));
}


void
NodalFloodCount::pack(std::vector<unsigned int> & packed_data) const
{
  // Don't repack the data if it's already packed - we might lose data that was updated
  // or stored into the packed_data that is not available in the local thread
  if (!packed_data.empty())
    return;

  std::vector<std::set<unsigned int> > data(_region_count+1);
  unsigned int n_periodic_nodes = 0;

  {
    std::map<unsigned int, int>::const_iterator end = _bubble_map.end();
    // Reorganize the data by values
    for (std::map<unsigned int, int>::const_iterator it = _bubble_map.begin(); it != end; ++it)
      data[(it->second)].insert(it->first);

    // Append our periodic neighbor nodes to the data structure before packing
    n_periodic_nodes = appendPeriodicNeighborNodes(data);

    mooseAssert(_region_count+1 == data.size(), "Error in packing data");
  }

  {
    /**
     * The size of the packed data structure should be the total number of marked
     * nodes plus inserted periodic neighbor information plus the number of unique bubbles.
     * This first resize will be an overesitmate because we aren't going to store nodes
     * marked with zeros (outside of bubbles).
     *
     * We will pack the data into a series of groups representing each unique bubble
     * the nodes for each group will be proceeded by the number of nodes in that group
     * [ <n_nodes> <n_0> <n_1> ... <n_n> <m_nodes> <n_0> <n_1> ... <n_m> ]
     */
    packed_data.resize(_bubble_map.size() + n_periodic_nodes + _region_count);

    // Now pack it up
    unsigned int current_idx = 0;

    // Note: The zeroth "region" is everything outside of a bubble - we don't want to put
    // that into our packed data structure so start at 1 here!
    for (unsigned int i=1 /* Yes - start at 1 */; i<=_region_count; ++i)
    {
      packed_data[current_idx++] = data[i].size();
      std::set<unsigned int>::iterator end = data[i].end();
      for (std::set<unsigned int>::iterator it = data[i].begin(); it != end; ++it)
        packed_data[current_idx++] = *it;
    }

    // Resize the structure to it's actual final size
    packed_data.resize(current_idx);
  }
}

void
NodalFloodCount::unpack(const std::vector<unsigned int> & packed_data)
{
  bool next_set = true;
  unsigned int curr_set_length=0;
  std::set<unsigned int> curr_set;

  _bubble_sets.clear();
  for (unsigned int i=0; i<packed_data.size(); ++i)
  {
    if (next_set)
    {
      if (i > 0)
      {
        _bubble_sets.push_back(curr_set);
        curr_set.clear();
      }

      // Get the length of the next set
      curr_set_length = packed_data[i];
    }
    else
    {
      // unpack each bubble
      curr_set.insert(packed_data[i]);
      --curr_set_length;
    }

    next_set = !(curr_set_length);
  }
  _bubble_sets.push_back(curr_set);

  mooseAssert(curr_set_length == 0, "Error in unpacking data");
}

void
NodalFloodCount::mergeSets()
{
  std::set<unsigned int> set_union;
  bool set_merged;

  /**
   * This loop will continue as long as sets of nodes are merged
   * creating "new" sets.  There may be other ways to optimize this loop
   * but it should not be a bottleneck in practice.
   */
  do
  {
    set_merged = false;
    std::list<std::set<unsigned int> >::iterator end = _bubble_sets.end();
    for (std::list<std::set<unsigned int> >::iterator it1 = _bubble_sets.begin(); it1 != end; ++it1)
    {
      std::list<std::set<unsigned int> >::iterator it2 = it1;
      ++it2; // Don't compare this set with itself - advance the iterator to the next set
      while (it2 != end)
      {
        set_union.clear();
        std::set_union(it1->begin(), it1->end(), it2->begin(), it2->end(),
                       std::inserter(set_union, set_union.end()));

        if (set_union.size() < it1->size() + it2->size())
        {
          *it1 = set_union;
          _bubble_sets.erase(it2++);
          set_merged = true;
        }
        else
          ++it2;
      }
    }
  } while (set_merged);
}

void
NodalFloodCount::flood(const Node *node, unsigned int region)
{
  if (node == NULL)
    return;

  unsigned int node_id = node->id();

  // Has this node already been marked? - if so move along
  if (_bubble_map.find(node_id) != _bubble_map.end())
    return;

  // This node hasn't been marked - is it in a bubble?
  if (_var.getNodalValue(*node) < _threshold)
  {
    // No - mark and return
    _bubble_map[node_id] = 0;
    return;
  }

  std::vector< const Node * > neighbors;
  MeshTools::find_nodal_neighbors(_mesh._mesh, *node, _nodes_to_elem_map, neighbors);
  // Important!  If this node doesn't have any neighbors (i.e. floating node in the center
  // of an element) we need to just unmark it for now since it can't be easiliy flooded
  if (neighbors.size() == 0)
  {
    _bubble_map[node_id] = 0;
    return;
  }

  // Yay! A bubble -> Mark it! (If region is zero that signifies that this is a new bubble)
  _bubble_map[node_id] = region ? region : ++_region_count;
  // If this is a periodic boudnary then there may be multiple corresponding nodes that need to
  // be marked depending on the number of mapped directions and node position - we'll do that now!

  for (unsigned int i=0; i<neighbors.size(); ++i)
  {
    // Only recurse on nodes this processor owns
    if (!region || isNodeValueValid(neighbors[i]->id()))
    {
      flood(neighbors[i], _bubble_map[node_id]);
    }
  }
}

bool
NodalFloodCount::isNodeValueValid(unsigned int node_id) const
{
  for (unsigned int j=0; j < _nodes_to_elem_map[node_id].size(); ++j)
    if (_nodes_to_elem_map[node_id][j]->processor_id() == libMesh::processor_id())
      return true;

  return false;
}

unsigned int
NodalFloodCount::appendPeriodicNeighborNodes(std::vector<std::set<unsigned int> > & data) const
{
  unsigned int inserted_counts = 0;

  /**
   * Now we will append our periodic neighbor information.  We treat the periodic neighbor nodes
   * much like we do ghosted nodes in a multi-processor setting.  If a bubble is sitting on a
   * periodic boundary we will simply add those periodic neighbors to the appropriate bubble
   * before packing up the data
   */
  for (unsigned int i = 0; i < data.size(); ++i)
  {
    std::set<unsigned int> periodic_neighbors;
    std::set<unsigned int> merged_sets;

    for (std::set<unsigned int>::iterator s_it = data[i].begin(); s_it != data[i].end(); ++s_it)
    {
      std::pair<std::multimap<unsigned int, unsigned int>::const_iterator, std::multimap<unsigned int, unsigned int>::const_iterator> iters =
        _periodic_node_map.equal_range(*s_it);
      for (std::multimap<unsigned int, unsigned int>::const_iterator it = iters.first; it != iters.second; ++it)
        periodic_neighbors.insert(it->second);
    }

    // Now that we have all of the periodic_neighbors in our temporary set we need to union it together with the original set
    // for the current bubble
    std::set_union(data[i].begin(), data[i].end(), periodic_neighbors.begin(), periodic_neighbors.end(), std::inserter(merged_sets, merged_sets.end()));

    inserted_counts += merged_sets.size() - data[i].size();
    // save the updated set back into our datastructure
    data[i] = merged_sets;
  }
  return inserted_counts;
}
