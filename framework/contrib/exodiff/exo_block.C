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
#include "exo_block.h"
#include "libmesh/exodusII.h"

#include <string>
#include <sstream>

using namespace std;

template <typename INT>
Exo_Block<INT>::Exo_Block()
  : Exo_Entity(),
    num_nodes_per_elmt(-1),
    conn(NULL)
{ }

template <typename INT>
Exo_Block<INT>::Exo_Block(int file_id, size_t exo_block_id)
  : Exo_Entity(file_id, exo_block_id),
    num_nodes_per_elmt(-1),
    conn(NULL)
{
  SMART_ASSERT(file_id >= 0);
  SMART_ASSERT((int)exo_block_id > EX_INVALID_ID);

  initialize(file_id, exo_block_id);
}

template <typename INT>
Exo_Block<INT>::Exo_Block(int file_id,
		     size_t id,
                     const char* type,
                     size_t num_e,
                     size_t num_npe)
  : Exo_Entity(file_id, id, num_e),
    elmt_type(type),
    num_nodes_per_elmt(num_npe),
    conn(NULL)
{
  SMART_ASSERT(id > 0);
  SMART_ASSERT(elmt_type != "");
  SMART_ASSERT(num_npe > 0);
}

template <typename INT>
Exo_Block<INT>::~Exo_Block()
{
  if (conn)  delete [] conn;
}

template <typename INT>
EXOTYPE Exo_Block<INT>::exodus_type() const {return EX_ELEM_BLOCK;}

template <typename INT>
void Exo_Block<INT>::entity_load_params()
{
  int num_attr;
  ex_block block;
  block.id = id_;
  block.type = EX_ELEM_BLOCK;
  int err = ex_get_block_param(fileId, &block);

  if (err < 0) {
    std::cout << "Exo_Block<INT>::Load_Block_Params(): ERROR: Failed to get element"
         << " block parameters!  Aborting..." << std::endl;
    exit(1);
  }

  numEntity = block.num_entry;
  num_nodes_per_elmt = block.num_nodes_per_entry;
  num_attr = block.num_attribute;
  elmt_type = block.topology;

  if (num_nodes_per_elmt < 0 || num_attr < 0)
  {
    std::cout << "Exo_Block<INT>::Load_Block_Params(): ERROR: Data appears corrupt for"
         << " block " << id_ << "(id=" << id_
         << ")!" << std::endl
         << "\tnum elmts = "          << numEntity  << std::endl
         << "\tnum nodes per elmt = " << num_nodes_per_elmt << std::endl
         << "\tnum attributes = "     << num_attr      << std::endl
         << " ... Aborting..." << std::endl;
    exit(1);
  }
}

template <typename INT>
string Exo_Block<INT>::Load_Connectivity()
{
  SMART_ASSERT(Check_State());

  if (fileId < 0) return "ERROR:  Invalid file id!";
  if (id_ == EX_INVALID_ID) return "ERROR:  Must initialize block parameters first!";

  if (conn) delete [] conn;  conn = 0;

  if (numEntity && num_nodes_per_elmt)
  {
    conn = new INT[ (size_t)numEntity * num_nodes_per_elmt ];  SMART_ASSERT(conn != 0);

    int err = ex_get_conn(fileId, EX_ELEM_BLOCK, id_, conn, 0, 0);
    if (err < 0) {
      std::cout << "Exo_Block<INT>::Load_Connectivity()  ERROR: Call to ex_get_conn"
           << " returned error value!  Block id = " << id_ << std::endl;
      std::cout << "Aborting..." << std::endl;
      exit(1);
    }
    else if (err > 0) {
      ostringstream oss;
      oss << "WARNING:  Number " << err
          << " returned from call to ex_get_conn()";
      return oss.str();
    }
  }

  return "";
}

template <typename INT>
string Exo_Block<INT>::Free_Connectivity()
{
  SMART_ASSERT(Check_State());
  if (conn) delete [] conn;  conn = 0;
  return "";
}

template <typename INT>
const INT* Exo_Block<INT>::Connectivity(size_t elmt_index) const
{
  SMART_ASSERT(Check_State());

  if (!conn || elmt_index >= numEntity) return 0;

  return &conn[(size_t)elmt_index * num_nodes_per_elmt];
}

template <typename INT>
string Exo_Block<INT>::Give_Connectivity(size_t& num_e, size_t& npe, INT*& recv_conn)
{
  if (num_nodes_per_elmt < 0)
    return "ERROR:  Connectivity parameters have not been determined!";

  num_e = numEntity;
  npe = num_nodes_per_elmt;
  recv_conn = conn;

  conn = 0;  // Transfers responsibility of deleting to the receiving pointer.

  return "";
}

template <typename INT>
int Exo_Block<INT>::Check_State() const
{
  SMART_ASSERT(id_ >= EX_INVALID_ID);
  SMART_ASSERT( !( id_ == EX_INVALID_ID && elmt_type != "" ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && num_nodes_per_elmt >= 0 ) );
  SMART_ASSERT( !( id_ == EX_INVALID_ID && conn ) );

  SMART_ASSERT( !( conn && (numEntity == 0 || num_nodes_per_elmt <= 0) ) );

  return 1;
}

template <typename INT>
void Exo_Block<INT>::Display_Stats(std::ostream& s) const
{
  s << "Exo_Block<INT>::Display()  block id = " << id_           << std::endl
    << "                  element type = " << elmt_type          << std::endl
    << "               number of elmts = " << numEntity          << std::endl
    << "      number of nodes per elmt = " << num_nodes_per_elmt << std::endl
    << "          number of attributes = " << attr_count()       << std::endl
    << "           number of variables = " << var_count()        << std::endl;
}

template <typename INT>
void Exo_Block<INT>::Display(std::ostream& s) const
{
  SMART_ASSERT(Check_State());

  s << "Exo_Block<INT>::Display()  block id = " << id_           << std::endl
    << "                  element type = " << elmt_type          << std::endl
    << "               number of elmts = " << numEntity          << std::endl
    << "      number of nodes per elmt = " << num_nodes_per_elmt << std::endl
    << "          number of attributes = " << attr_count()       << std::endl
    << "           number of variables = " << var_count()        << std::endl;

  if (conn) {
    size_t index = 0;
    s << "       connectivity = ";
    for (size_t e = 0; e < numEntity; ++e) {
      if (e != 0) s << "                      ";
      s << "(" << (e + 1) << ") ";
      for (int n = 0; n < num_nodes_per_elmt; ++n)
	s << conn[index++] << " ";
      s << std::endl;
    }
  }
}

template class Exo_Block<int>;
template class Exo_Block<int64_t>;
