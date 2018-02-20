// Copyright(C) 2008-2017 National Technology & Engineering Solutions
// of Sandia, LLC (NTESS).  Under the terms of Contract DE-NA0003525 with
// NTESS, the U.S. Government retains certain rights in this software.
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
//     * Neither the name of NTESS nor the names of its
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

#include "ED_SystemInterface.h" // for ERROR, SystemInterface, etc
#include "libmesh/exodusII.h"   // for ex_set, etc
#include "iqsort.h"             // for index_qsort
#include "node_set.h"
#include "smart_assert.h" // for SMART_ASSERT
#include <cstdlib>        // for exit
#include <iostream>       // for operator<<, ostream, etc
#include <vector>         // for vector

template <typename INT>
Node_Set<INT>::Node_Set()
  : Exo_Entity(), num_dist_factors(0), nodes(nullptr), nodeIndex(nullptr), dist_factors(nullptr)
{
}

template <typename INT>
Node_Set<INT>::Node_Set(int file_id, size_t id)
  : Exo_Entity(file_id, id),
    num_dist_factors(0),
    nodes(nullptr),
    nodeIndex(nullptr),
    dist_factors(nullptr)
{
}

template <typename INT>
Node_Set<INT>::Node_Set(int file_id, size_t id, size_t nnodes, size_t ndfs)
  : Exo_Entity(file_id, id, nnodes),
    num_dist_factors(ndfs),
    nodes(nullptr),
    nodeIndex(nullptr),
    dist_factors(nullptr)
{
}

template <typename INT>
Node_Set<INT>::~Node_Set()
{
  delete[] nodes;
  delete[] nodeIndex;
  delete[] dist_factors;
}

template <typename INT>
EXOTYPE
Node_Set<INT>::exodus_type() const
{
  return EX_NODE_SET;
}

template <typename INT>
const INT *
Node_Set<INT>::Nodes() const
{
  // See if already loaded...
  if (!nodes)
  {
    load_nodes();
  }
  return nodes;
}

template <typename INT>
size_t
Node_Set<INT>::Node_Id(size_t position) const
{
  if (numEntity <= 0)
  {
    return 0;
  }

  // See if already loaded...
  if (!nodes)
  {
    load_nodes();
  }
  SMART_ASSERT(position < numEntity);
  return nodes[nodeIndex[position]];
}

template <typename INT>
size_t
Node_Set<INT>::Node_Index(size_t position) const
{
  if (numEntity <= 0)
  {
    return 0;
  }

  // See if already loaded...
  if (!nodes)
  {
    load_nodes();
  }
  SMART_ASSERT(position < numEntity);
  SMART_ASSERT(nodeIndex != nullptr);
  return nodeIndex[position];
}

template <typename INT>
void
Node_Set<INT>::apply_map(const INT * node_map)
{
  SMART_ASSERT(node_map != nullptr);
  if (nodes != nullptr)
  {
    delete[] nodes;
    nodes = nullptr;
    delete[] nodeIndex;
    nodeIndex = nullptr;
  }
  load_nodes(node_map);
}

template <typename INT>
void
Node_Set<INT>::load_nodes(const INT * node_map) const
{
  if (numEntity > 0)
  {
    nodes = new INT[numEntity];
    SMART_ASSERT(nodes != nullptr);
    nodeIndex = new INT[numEntity];
    SMART_ASSERT(nodeIndex != nullptr);
    ex_get_set(fileId, EX_NODE_SET, id_, nodes, nullptr);

    if (node_map != nullptr)
    {
      for (size_t i = 0; i < numEntity; i++)
      {
        nodes[i] = 1 + node_map[nodes[i] - 1];
      }
    }

    for (size_t i = 0; i < numEntity; i++)
    {
      nodeIndex[i] = i;
    }
    if (interface.nsmap_flag)
    {
      index_qsort(nodes, nodeIndex, numEntity);
    }
  }
}

template <typename INT>
const double *
Node_Set<INT>::Distribution_Factors() const
{
  if ((dist_factors == nullptr) && num_dist_factors > 0)
  {
    dist_factors = new double[num_dist_factors];
    SMART_ASSERT(dist_factors != nullptr);
    ex_get_set_dist_fact(fileId, EX_NODE_SET, id_, dist_factors);
  }
  return dist_factors;
}

template <typename INT>
void
Node_Set<INT>::Free_Distribution_Factors() const
{
  if (dist_factors)
  {
    delete[] dist_factors;
    dist_factors = nullptr;
  }
}

template <typename INT>
void
Node_Set<INT>::Display(std::ostream & s)
{
  Check_State();
  s << "Node_Set<INT>::Display_Stats()  Exodus node set ID = " << id_ << '\n'
    << "                              number of nodes = " << numEntity << '\n'
    << "               number of distribution factors = " << num_dist_factors << '\n'
    << "                          number of variables = " << var_count() << '\n';
}

template <typename INT>
int
Node_Set<INT>::Check_State() const
{
  SMART_ASSERT(id_ >= EX_INVALID_ID);
  SMART_ASSERT(!(id_ == EX_INVALID_ID && numEntity > 0));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && num_dist_factors > 0));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && nodes));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && dist_factors));

  return 1;
}

template <typename INT>
void
Node_Set<INT>::entity_load_params()
{
  std::vector<ex_set> sets(1);
  sets[0].id = id_;
  sets[0].type = EX_NODE_SET;
  sets[0].entry_list = nullptr;
  sets[0].extra_list = nullptr;
  sets[0].distribution_factor_list = nullptr;

  int err = ex_get_sets(fileId, 1, &sets[0]);

  if (err < 0)
  {
    ERROR("Failed to get nodeset parameters for nodeset " << id_ << ". !  Aborting...\n");
    exit(1);
  }

  numEntity = sets[0].num_entry;
  num_dist_factors = sets[0].num_distribution_factor;
}

template class Node_Set<int>;
template class Node_Set<int64_t>;
