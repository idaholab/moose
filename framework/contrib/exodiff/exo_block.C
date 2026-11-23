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

#include "ED_SystemInterface.h" // for SystemInterface, interface
#include "exo_block.h"
#include "libmesh/exodusII.h" // for ex_block, etc
#include "smart_assert.h"     // for SMART_ASSERT
#include "stringx.h"          // for no_case_equals
#include <cstdlib>            // for exit, nullptr
#include <iostream>           // for operator<<, endl, ostream, etc
#include <numeric>            // for partial_sum
#include <string>             // for string, char_traits
#include <vector>

template <typename INT>
Exo_Block<INT>::Exo_Block() : Exo_Entity(), num_nodes_per_elmt(-1), is_nsided(false), conn(nullptr)
{
}

template <typename INT>
Exo_Block<INT>::Exo_Block(int file_id, size_t exo_block_id)
  : Exo_Entity(file_id, exo_block_id), num_nodes_per_elmt(-1), is_nsided(false), conn(nullptr)
{
  SMART_ASSERT(file_id >= 0);
  SMART_ASSERT((int)exo_block_id > EX_INVALID_ID);

  initialize(file_id, exo_block_id);
}

template <typename INT>
Exo_Block<INT>::Exo_Block(int file_id, size_t id, const char * type, size_t num_e, size_t num_npe)
  : Exo_Entity(file_id, id, num_e),
    elmt_type(type),
    num_nodes_per_elmt(num_npe),
    is_nsided(no_case_equals(elmt_type, "nsided")),
    conn(nullptr)
{
  SMART_ASSERT(id > 0);
  SMART_ASSERT(elmt_type != "");
  SMART_ASSERT(num_npe > 0);
}

template <typename INT>
Exo_Block<INT>::~Exo_Block()
{
  if (conn)
  {
    delete[] conn;
  }
}

template <typename INT>
EXOTYPE
Exo_Block<INT>::exodus_type() const
{
  return EX_ELEM_BLOCK;
}

template <typename INT>
void
Exo_Block<INT>::entity_load_params()
{
  int num_attr;
  ex_block block{};
  block.id = id_;
  block.type = EX_ELEM_BLOCK;
  int err = ex_get_block_param(fileId, &block);

  if (err < 0)
  {
    ERROR("Exo_Block<INT>::Load_Block_Params(): Failed to get element"
          << " block parameters!  Aborting...\n");
    exit(1);
  }

  numEntity = block.num_entry;
  num_nodes_per_elmt = block.num_nodes_per_entry;
  num_attr = block.num_attribute;
  elmt_type = block.topology;
  // For an "NSIDED" block, num_nodes_per_entry above is the total over the block rather than a
  // per-element stride; the per-element counts come from ex_get_entity_count_per_polyhedra().
  is_nsided = no_case_equals(elmt_type, "nsided");

  if (num_nodes_per_elmt < 0 || num_attr < 0)
  {
    ERROR("Exo_Block<INT>::Load_Block_Params(): Data appears corrupt for"
          << " block " << id_ << "(id=" << id_ << ")!\n"
          << "\tnum elmts = " << numEntity << '\n'
          << "\tnum nodes per elmt = " << num_nodes_per_elmt << '\n'
          << "\tnum attributes = " << num_attr << '\n'
          << " ... Aborting...\n");
    exit(1);
  }
}

template <typename INT>
std::string
Exo_Block<INT>::Load_Connectivity()
{
  SMART_ASSERT(Check_State());

  if (fileId < 0)
  {
    return "ERROR:  Invalid file id!";
  }
  if (id_ == EX_INVALID_ID)
  {
    return "ERROR:  Must initialize block parameters first!";
  }
  if (conn)
  {
    delete[] conn;
  }
  conn = nullptr;
  conn_offset.clear();

  if (numEntity && num_nodes_per_elmt)
  {
    // For an "NSIDED" block num_nodes_per_elmt is already the total over the block, so it is the
    // full connectivity length; for a regular block it is the per-element stride.
    conn = new INT[is_nsided ? num_nodes_per_elmt : numEntity * num_nodes_per_elmt];
    SMART_ASSERT(conn != nullptr);

    int err = ex_get_conn(fileId, EX_ELEM_BLOCK, id_, conn, nullptr, nullptr);
    if (err < 0)
    {
      ERROR("Exo_Block<INT>::Load_Connectivity(): Call to ex_get_conn"
            << " returned error value!  Block id = " << id_ << '\n'
            << "Aborting...\n");
      exit(1);
    }
    else if (err > 0)
    {
      std::ostringstream oss;
      oss << "WARNING:  Number " << err << " returned from call to ex_get_conn()";
      return oss.str();
    }

    if (is_nsided)
    {
      std::vector<int> node_counts(numEntity);
      err = ex_get_entity_count_per_polyhedra(fileId, EX_ELEM_BLOCK, id_, node_counts.data());
      if (err < 0)
      {
        ERROR("Exo_Block<INT>::Load_Connectivity(): Call to"
              << " ex_get_entity_count_per_polyhedra returned error value!  Block id = " << id_
              << '\n'
              << "Aborting...\n");
        exit(1);
      }

      // Exclusive prefix sum of the per-element node counts, with a trailing total.
      conn_offset.resize(numEntity + 1);
      conn_offset[0] = 0;
      std::partial_sum(node_counts.begin(), node_counts.end(), conn_offset.begin() + 1);

      // The per-element counts and the block total come from separate Exodus arrays, and every
      // access below indexes conn through the offsets derived from the former, so a disagreement
      // would silently read out of bounds.
      if (conn_offset[numEntity] != static_cast<size_t>(num_nodes_per_elmt))
      {
        ERROR("Exo_Block<INT>::Load_Connectivity(): Block id = "
              << id_ << " node counts sum to " << conn_offset[numEntity]
              << ", but the block declares " << num_nodes_per_elmt << " nodes!\n"
              << "Aborting...\n");
        exit(1);
      }
    }
  }

  return "";
}

template <typename INT>
std::string
Exo_Block<INT>::Free_Connectivity()
{
  SMART_ASSERT(Check_State());
  if (conn)
  {
    delete[] conn;
  }
  conn = nullptr;
  conn_offset.clear();
  return "";
}

template <typename INT>
const INT *
Exo_Block<INT>::Connectivity(size_t elmt_index) const
{
  SMART_ASSERT(Check_State());

  if (!conn || elmt_index >= numEntity)
  {
    return nullptr;
  }

  return &conn[is_nsided ? conn_offset[elmt_index] : elmt_index * num_nodes_per_elmt];
}

template <typename INT>
size_t
Exo_Block<INT>::Num_Nodes_per_Elmt(size_t elmt_index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(elmt_index < numEntity);

  if (!is_nsided)
  {
    return num_nodes_per_elmt;
  }

  // The offsets are only built by Load_Connectivity(), so the per-element counts of a polygon
  // block are not known until its connectivity has been read.
  SMART_ASSERT(!conn_offset.empty());
  return conn_offset[elmt_index + 1] - conn_offset[elmt_index];
}

template <typename INT>
std::string
Exo_Block<INT>::Give_Connectivity(size_t & num_e, size_t & npe, INT *& recv_conn)
{
  if (num_nodes_per_elmt < 0)
  {
    return "ERROR:  Connectivity parameters have not been determined!";
  }
  num_e = numEntity;
  npe = num_nodes_per_elmt;
  recv_conn = conn;

  conn = nullptr; // Transfers responsibility of deleting to the receiving pointer.

  return "";
}

template <typename INT>
int
Exo_Block<INT>::Check_State() const
{
  SMART_ASSERT(id_ >= EX_INVALID_ID);
  SMART_ASSERT(!(id_ == EX_INVALID_ID && elmt_type != ""));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && num_nodes_per_elmt >= 0));
  SMART_ASSERT(!(id_ == EX_INVALID_ID && conn));

  SMART_ASSERT(!(conn && (numEntity == 0 || num_nodes_per_elmt <= 0)));

  return 1;
}

template <typename INT>
void
Exo_Block<INT>::Display_Stats(std::ostream & s) const
{
  s << "Exo_Block<INT>::Display()  block id = " << id_ << '\n'
    << "                  element type = " << elmt_type << '\n'
    << "               number of elmts = " << numEntity << '\n'
    << "      number of nodes per elmt = " << num_nodes_per_elmt << '\n'
    << "          number of attributes = " << attr_count() << '\n'
    << "           number of variables = " << var_count() << '\n';
}

template <typename INT>
void
Exo_Block<INT>::Display(std::ostream & s) const
{
  SMART_ASSERT(Check_State());

  s << "Exo_Block<INT>::Display()  block id = " << id_ << '\n'
    << "                  element type = " << elmt_type << '\n'
    << "               number of elmts = " << numEntity << '\n'
    << "      number of nodes per elmt = " << num_nodes_per_elmt << '\n'
    << "          number of attributes = " << attr_count() << '\n'
    << "           number of variables = " << var_count() << '\n';

  if (conn)
  {
    s << "       connectivity = ";
    for (size_t e = 0; e < numEntity; ++e)
    {
      if (e != 0)
      {
        s << "                      ";
      }
      s << "(" << (e + 1) << ") ";
      const INT * elmt_conn = Connectivity(e);
      for (size_t n = 0; n < Num_Nodes_per_Elmt(e); ++n)
      {
        s << elmt_conn[n] << " ";
      }
      s << '\n';
    }
  }
}

template class Exo_Block<int>;
template class Exo_Block<int64_t>;
