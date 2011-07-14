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
#include "exodusII.h"

#include <string>
#include <sstream>

using namespace std;

Exo_Block::Exo_Block()
  : Exo_Entity(),
    num_nodes_per_elmt(-1),
    conn(NULL)
{ }

Exo_Block::Exo_Block(int file_id, int exo_block_id)
  : Exo_Entity(file_id, exo_block_id),
    num_nodes_per_elmt(-1),
    conn(NULL)
{
  SMART_ASSERT(file_id >= 0);
  SMART_ASSERT(exo_block_id > 0);
  
  initialize(file_id, exo_block_id);
}

Exo_Block::Exo_Block(int file_id,
		     int id,
                     const char* type,
                     int num_e,
                     int num_npe)
  : Exo_Entity(file_id, id, num_e),
    elmt_type(type),
    num_nodes_per_elmt(num_npe),
    conn(NULL)
{
  SMART_ASSERT(id > 0);
  SMART_ASSERT(elmt_type != "");
  SMART_ASSERT(num_e >= 0);
  SMART_ASSERT(num_npe > 0);
}

Exo_Block::~Exo_Block()
{
  if (conn)  delete [] conn;
}

EXOTYPE Exo_Block::exodus_type() const {return EX_ELEM_BLOCK;}

void Exo_Block::entity_load_params()
{
  char eltype[MAX_STR_LENGTH+1];
  int num_attr;
  int err = ex_get_block(fileId, EX_ELEM_BLOCK, id_, eltype, &numEntity,
			 &num_nodes_per_elmt, 0, 0, &num_attr);
  
  if (err < 0) {
    std::cout << "Exo_Block::Load_Block_Params(): ERROR: Failed to get element"
         << " block parameters!  Aborting..." << std::endl;
    exit(1);
  }
  
  if (numEntity < 0 ||
      num_nodes_per_elmt < 0 ||
      num_attr < 0)
  {
    std::cout << "Exo_Block::Load_Block_Params(): ERROR: Data appears corrupt for"
         << " block " << id_ << "(id=" << id_
         << ")!" << std::endl
         << "\tnum elmts = "          << numEntity  << std::endl
         << "\tnum nodes per elmt = " << num_nodes_per_elmt << std::endl
         << "\tnum attributes = "     << num_attr      << std::endl
         << " ... Aborting..." << std::endl;
    exit(1);
  }
  elmt_type = eltype;
}

string Exo_Block::Load_Connectivity()
{
  SMART_ASSERT(Check_State());
  
  if (fileId < 0) return "ERROR:  Invalid file id!";
  if (id_ == 0) return "ERROR:  Must initialize block parameters first!";
  
  if (conn) delete [] conn;  conn = 0;
  
  if (numEntity && num_nodes_per_elmt)
  {
    conn = new int[ (size_t)numEntity * num_nodes_per_elmt ];  SMART_ASSERT(conn != 0);
    
    int err = ex_get_conn(fileId, EX_ELEM_BLOCK, id_, conn, 0, 0);
    if (err < 0) {
      std::cout << "Exo_Block::Load_Connectivity()  ERROR: Call to ex_get_conn"
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

string Exo_Block::Free_Connectivity()
{
  SMART_ASSERT(Check_State());
  if (conn) delete [] conn;  conn = 0;
  return "";
}

const int* Exo_Block::Connectivity(int elmt_index) const
{
  SMART_ASSERT(Check_State());
  
  if (!conn || elmt_index < 0 || elmt_index >= numEntity) return 0;
  
  return &conn[elmt_index * num_nodes_per_elmt];
}

string Exo_Block::Give_Connectivity(int& num_e, int& npe, int*& recv_conn)
{
  if (numEntity < 0 || num_nodes_per_elmt < 0)
    return "ERROR:  Connectivity parameters have not been determined!";
  
  num_e = numEntity;
  npe = num_nodes_per_elmt;
  recv_conn = conn;
  
  conn = 0;  // Transfers responsibility of deleting to the receiving pointer.
  
  return "";
}

int Exo_Block::Check_State() const
{
  SMART_ASSERT(id_ >= 0);
  SMART_ASSERT(index_ >= -1);
  SMART_ASSERT(numEntity >= -1);
  SMART_ASSERT(num_nodes_per_elmt >= -1);
  
  SMART_ASSERT( !( id_ == 0 && elmt_type != "" ) );
  SMART_ASSERT( !( id_ == 0 && numEntity > -1 ) );
  SMART_ASSERT( !( id_ == 0 && num_nodes_per_elmt > -1 ) );
  SMART_ASSERT( !( id_ == 0 && conn ) );
  
  SMART_ASSERT( !( conn && (numEntity <= 0 || num_nodes_per_elmt <= 0) ) );
  
  return 1;
}

void Exo_Block::Display_Stats(std::ostream& s) const
{
  s << "Exo_Block::Display()  block id = " << id_           << std::endl
    << "                  element type = " << elmt_type          << std::endl
    << "               number of elmts = " << numEntity          << std::endl
    << "      number of nodes per elmt = " << num_nodes_per_elmt << std::endl
    << "          number of attributes = " << attr_count()       << std::endl
    << "           number of variables = " << var_count()        << std::endl;
}

void Exo_Block::Display(std::ostream& s) const
{
  SMART_ASSERT(Check_State());
  
  s << "Exo_Block::Display()  block id = " << id_           << std::endl
    << "                  element type = " << elmt_type          << std::endl
    << "               number of elmts = " << numEntity          << std::endl
    << "      number of nodes per elmt = " << num_nodes_per_elmt << std::endl
    << "          number of attributes = " << attr_count()       << std::endl
    << "           number of variables = " << var_count()        << std::endl;
  
  if (conn) {
    int index = 0;
    s << "       connectivity = ";
    for (int e = 0; e < numEntity; ++e) {
      if (e != 0) s << "                      ";
      s << "(" << (e + 1) << ") ";
      for (int n = 0; n < num_nodes_per_elmt; ++n)
	s << conn[index++] << " ";
      s << std::endl;
    }
  }
}
