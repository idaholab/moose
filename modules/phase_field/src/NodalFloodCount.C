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

#include "mesh_tools.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "dof_map.h"

#include <algorithm>
#include <limits>

template<>
InputParameters validParams<NodalFloodCount>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addRequiredParam<std::string>("variable", "The variable to be monitored");
  params.addParam<Real>("threshold", 0.5, "The threshold value of the bubble boundary");
  return params;
}

NodalFloodCount::NodalFloodCount(const std::string & name, InputParameters parameters) :
  ElementPostprocessor(name, parameters),
  _threshold(getParam<Real>("threshold")),
  _mesh(_subproblem.mesh()),
  _moose_var(_subproblem.getVariable(0, getParam<std::string>("variable"))),
  _var_number(_moose_var.number()),
  _region_count(0)
  // DEBUG
//  _dof_map(static_cast<FEProblem &>(_subproblem).getNonlinearSystem().dofMap())
{}

void
NodalFloodCount::initialize()
{
  // Clear the bubble marking map
  _bubble_map.clear();

  // Reset the packed data structure
  _packed_data.clear();

  // Reset the region counter
  _region_count = 0;

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh._mesh, _nodes_to_elem_map);
}

void
NodalFloodCount::execute()
{
  unsigned int n_nodes = _current_elem->n_nodes();
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

  // Now we need to merge the sets to remove bubbles that were partially counted by multiple processors
  // This call consolidates _bubble_sets
  mergeSets();

  //DEBUG
 //  std::cout << "\n\n************* AFTER MERGE ********************\n";
//   unsigned int ret_value = _bubble_sets.size();
//   while (!_bubble_sets.empty())
//   {
//     std::list<std::set<unsigned int> >::iterator low;
//     unsigned int min = std::numeric_limits<unsigned int>::max();
//     for (std::list<std::set<unsigned int> >::iterator li = _bubble_sets.begin(); li != _bubble_sets.end(); ++li)
//     {
//       if (*(li->begin()) < min)
//       {
//         min = *(li->begin());
//         low = li;
//       }
//     }
//     std::cout << "\nL " << low->size() << "\n";
//     unsigned int counter=5;
//     for (std::set<unsigned int>::iterator it = low->begin(); it != low->end(); ++it)
//     {
//       std::cout << *it << " ";
// //      if (--counter == 0) break;
//     }
//     _bubble_sets.erase(low);
//  }
  //DEBUG

  // Finally return the number of bubbles (count of unique sets)
//  return ret_value;
  return _bubble_sets.size();
//  return _region_count;
}

void
NodalFloodCount::threadJoin(const Postprocessor &y)
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

  {
    std::map<unsigned int, int>::const_iterator end = _bubble_map.end();
    // Reorganize the data by values
    for (std::map<unsigned int, int>::const_iterator it = _bubble_map.begin(); it != end; ++it)
      data[(it->second)].insert(it->first);

    mooseAssert(_region_count+1 == data.size(), "Error in packing data");
  }

  //DEBUG
//   for (unsigned int j=1; j<data.size(); ++j)
//   {
//     std::cout << "\nL " << data[j].size() << "\n";
//     for (std::set<unsigned int>::iterator it = data[j].begin(); it != data[j].end(); ++it)
//     {
//       std::cout << *it << " ";
//     }
//   }
  //DEBUG

  {
    /**
     * The size of the packed data structure should be the total number of marked
     * nodes plus the number of unique bubbles.
     *
     * We will pack the data into a series of groups representing each unique bubble
     * the nodes for each group will be proceeded by the number of nodes in that group
     * [ <n_nodes> <n_0> <n_1> ... <n_n> <m_nodes> <n_0> <n_1> ... <n_m> ]
     */
    packed_data.resize(_bubble_map.size() + _region_count);

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

    packed_data.resize(current_idx);
  }
}

void
NodalFloodCount::unpack(const std::vector<unsigned int> & packed_data)
{
  bool next_set = true;
  unsigned int curr_set_length;
  std::set<unsigned int> curr_set;

  //DEBUG
//   std::cout << "\npacked data:\n";
//   for (unsigned int j=0; j<packed_data.size(); ++j)
//   {
//     std::cout << packed_data[j] << " ";
//   }
//   std::cout << std::endl;
  //DEBUG

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

        // If the union of these two sets is less than
//      std::cout << set_union.size() << " < " << it1->size() << " + " <<  it2->size() << std::endl;
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

  // Get the value from the serialized solution
  // zeroeth system (Nonlinear), and zeroeth component (Scalar)
//  unsigned int dof_number = node->dof_number(0, _var_number, 0);

  // This node hasn't been marked - is it in a bubble?
//  if (_serialized_solution(dof_number) < _threshold)
  if (_moose_var.getNodalValue(*node) < _threshold)
  {
    // No - mark and return
    _bubble_map[node_id] = 0;
    return;
  }

  // Mark it! (If region is zero that signifies that this is a new bubble)
  _bubble_map[node_id] = region ? region : ++_region_count;

  // Yay! A bubble
  std::vector< const Node * > neighbors;
  MeshTools::find_nodal_neighbors(_mesh._mesh, *node, _nodes_to_elem_map, neighbors);
  for (unsigned int i=0; i<neighbors.size(); ++i)
  {
    // Only recurse one nodes this processor owns
    if (!region || neighbors[i]->processor_id() == libMesh::processor_id())
    {
      //DEBUG
//       std::vector<unsigned int> dof_indices(1);
//       dof_indices[0] = neighbors[i]->dof_number(0, _var_number, 0);
//       if (!_dof_map.all_semilocal_indices(dof_indices))
//         mooseError("Hmm - doesn't look like we can safely access the value at this node\n");
      //DEBUG
      flood(neighbors[i], _bubble_map[node_id]);
//    else
//      std::cout << "Not recursing on Node: " << node_id << " since we do not own it\n";
    }
  }
}
