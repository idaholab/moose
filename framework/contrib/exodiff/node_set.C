// Copyright(C) 2008 Sandia Corporation.  Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <iostream>
#include <cstdlib>

#include "smart_assert.h"
#include "node_set.h"
#include "libmesh/exodusII.h"
#include "iqsort.h"
#include "ED_SystemInterface.h"


using namespace std;

template <typename INT>
Node_Set<INT>::Node_Set()
  : Exo_Entity(),
    num_dist_factors(0),
    nodes(NULL),
    nodeIndex(NULL),
    dist_factors(NULL)
{ }

template <typename INT>
Node_Set<INT>::Node_Set(int file_id, size_t id)
  : Exo_Entity(file_id, id),
    num_dist_factors(0),
    nodes(NULL),
    nodeIndex(NULL),
    dist_factors(NULL)
{ }

template <typename INT>
Node_Set<INT>::Node_Set(int file_id, size_t id, size_t nnodes, size_t ndfs)
  : Exo_Entity(file_id, id, nnodes),
    num_dist_factors(ndfs),
    nodes(NULL),
    nodeIndex(NULL),
    dist_factors(NULL)
{}

template <typename INT>
Node_Set<INT>::~Node_Set()
{
  delete [] nodes;
  delete [] nodeIndex;
  delete [] dist_factors;
}

template <typename INT>
EXOTYPE Node_Set<INT>::exodus_type() const {return EX_NODE_SET;}

template <typename INT>
const INT* Node_Set<INT>::Nodes() const
{
  // See if already loaded...
  if (!nodes) {
    load_nodes();
  }
  return nodes;
}

template <typename INT>
size_t Node_Set<INT>::Node_Id(size_t position) const
{
  if (numEntity <= 0) {
    return 0;
  } else {
    // See if already loaded...
    if (!nodes) {
      load_nodes();
    }
    SMART_ASSERT(position < numEntity);
    return nodes[nodeIndex[position]];
  }
}

template <typename INT>
size_t Node_Set<INT>::Node_Index(size_t position) const
{
  if (numEntity <= 0) {
    return 0;
  } else {
    // See if already loaded...
    if (!nodes) {
      load_nodes();
    }
    SMART_ASSERT(position < numEntity);
    SMART_ASSERT(nodeIndex != NULL);
    return nodeIndex[position];
  }
}

template <typename INT>
void Node_Set<INT>::apply_map(const INT *node_map)
{
  SMART_ASSERT(node_map != NULL);
  if (nodes != NULL) {
    delete [] nodes;     nodes = NULL;
    delete [] nodeIndex; nodeIndex = NULL;
  }
  load_nodes(node_map);
}

template <typename INT>
void Node_Set<INT>::load_nodes(const INT *node_map) const
{
  if (numEntity > 0) {
    nodes = new INT[numEntity];  SMART_ASSERT(nodes != 0);
    nodeIndex = new INT[numEntity];  SMART_ASSERT(nodeIndex != 0);
    ex_get_set(fileId, EX_NODE_SET, id_, nodes, 0);

    if (node_map != NULL) {
      for (size_t i=0; i < numEntity; i++) {
	nodes[i] = 1+node_map[nodes[i]-1];
      }
    }

    for (size_t i=0; i < numEntity; i++) {
      nodeIndex[i] = i;
    }
    if (interface.nsmap_flag)
      index_qsort(nodes, nodeIndex, numEntity);
  }
}

template <typename INT>
const double* Node_Set<INT>::Distribution_Factors() const
{
  if (!dist_factors && num_dist_factors > 0) {
    dist_factors = new double[num_dist_factors];  SMART_ASSERT(dist_factors != 0);
    ex_get_set_dist_fact(fileId, EX_NODE_SET, id_, dist_factors);
  }
  return dist_factors;
}

template <typename INT>
void Node_Set<INT>::Display(std::ostream& s)
{
  Check_State();
  s << "Node_Set<INT>::Display_Stats()  Exodus node set ID = " << id_              << std::endl
    << "                              number of nodes = " << numEntity        << std::endl
    << "               number of distribution factors = " << num_dist_factors << std::endl
    << "                          number of variables = " << var_count()      << std::endl;
}

template <typename INT>
int Node_Set<INT>::Check_State() const
{
  SMART_ASSERT(id_ >= EX_INVALID_ID);
  SMART_ASSERT( !( id_ == EX_INVALID_ID && numEntity > 0 ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && num_dist_factors > 0 ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && nodes ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && dist_factors ) );

  return 1;
}

template <typename INT>
void Node_Set<INT>::entity_load_params()
{
  std::vector<ex_set> sets(1);
  sets[0].id = id_;
  sets[0].type = EX_NODE_SET;
  sets[0].entry_list = NULL;
  sets[0].extra_list = NULL;
  sets[0].distribution_factor_list = NULL;

  int err = ex_get_sets(fileId, 1, &sets[0]);

  if (err < 0) {
    std::cout << "ERROR: Failed to get nodeset parameters for nodeset " << id_
	      << ". !  Aborting..." << std::endl;
    exit(1);
  }

  numEntity = sets[0].num_entry;
  num_dist_factors = sets[0].num_distribution_factor;
}

template class Node_Set<int>;
template class Node_Set<int64_t>;
