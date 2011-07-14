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
#include "exoII_read.h"
#include "exo_block.h"
#include "node_set.h"
#include "side_set.h"
#include "exodusII.h"

#include <string>
#include <sstream>

using namespace std;

ExoII_Read::ExoII_Read()
  : file_id(-1),                 // value of -1 indicates file not open
    num_nodes(0),
    dimension(0),
    num_elmts(0),
    num_elmt_blocks(0),
    num_node_sets(0),
    num_side_sets(0),
    db_version(0.0),
    api_version(0.0),
    io_word_size(0),
    eblocks(0),
    nsets(0),
    ssets(0),
    nodes(0),
    node_map(0),
    elmt_map(0),
    elmt_order(0),
    num_times(0),
    times(0),
    cur_time(0),
    results(0),
    global_vals(0)
{ }

ExoII_Read::ExoII_Read(const char* fname)
  : file_name(fname),
    file_id(-1),                 // value of -1 indicates file not open
    num_nodes(0),
    dimension(0),
    num_elmts(0),
    num_elmt_blocks(0),
    num_node_sets(0),
    num_side_sets(0),
    db_version(0.0),
    api_version(0.0),
    io_word_size(0),
    eblocks(0),
    nsets(0),
    ssets(0),
    nodes(0),
    node_map(0),
    elmt_map(0),
    elmt_order(0),
    num_times(0),
    times(0),
    cur_time(0),
    results(0),
    global_vals(0)
{ }

ExoII_Read::~ExoII_Read()
{
  try {
    SMART_ASSERT(Check_State());
  
    if (file_id >= 0)
      {
	string err = Close_File();
	if (!err.empty())
	  std::cout << "ExoII_Read destructor()  ERROR closing file:"
		    << " \"" << err << "\"" << std::endl;
      }
  
    delete [] eblocks;
    delete [] nsets;
    delete [] ssets;
    delete [] nodes;
    delete [] times;
    if (results) {
      for (unsigned i = 0; i < nodal_vars.size(); ++i)
	delete [] results[i];
      delete [] results;
    }
    delete [] global_vals;
    delete [] node_map;
    delete [] elmt_map;
    delete [] elmt_order;
  } catch (...) {
  }
}

string ExoII_Read::Close_File()
{
  SMART_ASSERT(Check_State());
  
  if (file_id < 0) return "ERROR: File is not open!";
  
  int err = ex_close(file_id);
  
  if (err < 0) {
    std::cout << "ExoII_Read::Close_File(): ERROR " << err
         << ": Unable to close file!  Aborting..." << std::endl;
    exit(1);
  }
  if (err > 0) {
    ostringstream oss;
    oss << "WARNING: " << err << " issued upon close";
    return oss.str();
  }
  
  file_id = -1;
  
  return "";
}

// **********************  Access functions  ************************ //

string ExoII_Read::Coordinate_Name(unsigned i) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(i < 3);
  
  if (i < coord_names.size())
    return coord_names[i];
  else
    return "";
}

double ExoII_Read::Time(int time_num) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(time_num > 0 && time_num <= num_times);
  return times[time_num-1];
}

const string& ExoII_Read::Global_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < global_vars.size());
  return global_vars[index];
}

const string& ExoII_Read::Nodal_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < nodal_vars.size());
  return nodal_vars[index];
}

const string& ExoII_Read::Elmt_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < elmt_vars.size());
  return elmt_vars[index];
}

const string& ExoII_Read::Elmt_Att_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < elmt_atts.size());
  return elmt_atts[index];
}

const string& ExoII_Read::NS_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < ns_vars.size());
  return ns_vars[index];
}

const string& ExoII_Read::SS_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < ss_vars.size());
  return ss_vars[index];
}

Exo_Block* ExoII_Read::Get_Elmt_Block_by_Index(int block_index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(block_index >= 0 && block_index < num_elmt_blocks);
  return &eblocks[block_index];
}

Exo_Block* ExoII_Read::Get_Elmt_Block_by_Id(int id) const
{
  SMART_ASSERT(Check_State());
  for (int i=0; i < num_elmt_blocks; i++) {
    if (eblocks[i].Id() == id) {
      return &eblocks[i];
    }
  }
  return NULL;
}

Exo_Entity* ExoII_Read::Get_Entity_by_Index(EXOTYPE type, int block_index) const
{
  SMART_ASSERT(Check_State());
  
  switch (type) {
  case EX_ELEM_BLOCK:
    SMART_ASSERT(block_index >= 0 && block_index < num_elmt_blocks);
    return &eblocks[block_index];
  case EX_NODE_SET:
    SMART_ASSERT(block_index >= 0 && block_index < num_node_sets);
    return &nsets[block_index];
  case EX_SIDE_SET:
    SMART_ASSERT(block_index >= 0 && block_index < num_side_sets);
    return &ssets[block_index];
  default:
    return NULL;
  }
  return NULL;
}

Exo_Entity* ExoII_Read::Get_Entity_by_Id(EXOTYPE type, int id) const
{
  SMART_ASSERT(Check_State());
  switch (type) {
  case EX_ELEM_BLOCK:
    for (int i=0; i < num_elmt_blocks; i++) {
      if (eblocks[i].Id() == id) {
	return &eblocks[i];
      }
    }
    break;
  case EX_NODE_SET:
    for (int i=0; i < num_node_sets; i++) {
      if (nsets[i].Id() == id) {
	return &nsets[i];
      }
    }
    break;
  case EX_SIDE_SET:
    for (int i=0; i < num_side_sets; i++) {
      if (ssets[i].Id() == id) {
	return &ssets[i];
      }
    }
    break;
  default:
   return NULL;
  }
  return NULL;
}

Node_Set* ExoII_Read::Get_Node_Set_by_Id(int set_id) const
{
  SMART_ASSERT(Check_State());
  for (int i=0; i < num_node_sets; i++) {
    if (nsets[i].Id() == set_id) {
      return &nsets[i];
    }
  }
  return NULL;
}

Side_Set* ExoII_Read::Get_Side_Set_by_Id(int set_id) const
{
  SMART_ASSERT(Check_State());
  for (int i=0; i < num_side_sets; i++) {
    if (ssets[i].Id() == set_id) {
      return &ssets[i];
    }
  }
  return NULL;
}

string ExoII_Read::Load_Elmt_Block_Description(int block_index) const
{
  SMART_ASSERT(Check_State());
  if (!Open()) return "ERROR:  Must open file before loading blocks!";
  
  SMART_ASSERT(block_index >= 0 && block_index < num_elmt_blocks);
  
  eblocks[block_index].Load_Connectivity();
//  eblocks[idx].Load_Connectivity();
//  eblocks[idx].Load_Attributes();
  
  return "";
}

string ExoII_Read::Load_Elmt_Block_Descriptions() const
{
  SMART_ASSERT(Check_State());
  if (!Open()) return "ERROR:  Must open file before loading blocks!";
  
  for (int b = 0; b < num_elmt_blocks; ++b)
  {
    eblocks[b].Load_Connectivity();
//    eblocks[b].Load_Attributes();
  }
  
  return "";
}

string ExoII_Read::Free_Elmt_Block(int block_index) const
{
  SMART_ASSERT(Check_State());
  
  SMART_ASSERT(block_index >= 0 && block_index < num_elmt_blocks);
  
  eblocks[block_index].Free_Connectivity();
  eblocks[block_index].Free_Attributes();
//  eblocks[idx].Free_Connectivity();
//  eblocks[idx].Free_Attributes();
  
  return "";
}

string ExoII_Read::Free_Elmt_Blocks() const
{
  SMART_ASSERT(Check_State());
  
  for (int b = 0; b < num_elmt_blocks; ++b)
  {
    eblocks[b].Free_Connectivity();
    eblocks[b].Free_Attributes();
  }
  
  return "";
}

string ExoII_Read::Give_Connectivity(int block_index,
                                     int& num_e,
                                     int& npe,
                                     int*& new_conn)
{
  SMART_ASSERT(block_index >= 0 && block_index < num_elmt_blocks);
  
  return eblocks[block_index].Give_Connectivity(num_e, npe, new_conn);
}
  

int ExoII_Read::Block_Id(int block_index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(block_index >= 0 && block_index < num_elmt_blocks);
  return eblocks[block_index].Id();
}

int ExoII_Read::Block_Index(int block_id) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(block_id >= 0);
  for (int b = 0; b < num_elmt_blocks; ++b)
    if (eblocks[b].Id() == block_id)
      return b;
  return -1;
}

int ExoII_Read::Node_Set_Index(int id) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(id >= 0);
  for (int b = 0; b < num_node_sets; ++b)
    if (nsets[b].Id() == id)
      return b;
  return -1;
}

int ExoII_Read::Side_Set_Index(int id) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(id >= 0);
  for (int b = 0; b < num_side_sets; ++b)
    if (ssets[b].Id() == id)
      return b;
  return -1;
}

string ExoII_Read::Load_Node_Map()
{
  SMART_ASSERT(Check_State());
  
  if (!Open()) return "WARNING:  File not open!";
  
  delete [] node_map;
  node_map = 0;
  
  if (num_nodes == 0) return "WARNING:  There are no nodes!";
  
  node_map = new int[ num_nodes ];  SMART_ASSERT(node_map != 0);
  
  ex_opts(0);  // Temporarily turn off error reporting in case map isn't stored.
  int err = ex_get_node_num_map(file_id, node_map);
  ex_opts(EX_VERBOSE);
  
  if (err < 0) {
    std::cout << "ExoII_Read::Load_Node_Map()  ERROR: Unable to load node map; "
         << "Exodus error = " << err << ".  Aborting..." << std::endl;
    exit(1);
  }
  else if (err > 0)
    return "WARNING: Default node map being used.";
  
  return "";
}

string ExoII_Read::Free_Node_Map()
{
  SMART_ASSERT(Check_State());
  
  delete [] node_map;
  node_map = 0;
  
  return "";
}

string ExoII_Read::Load_Elmt_Map()
{
  SMART_ASSERT(Check_State());
  
  if (!Open()) return "WARNING:  File not open!";
  
  delete [] elmt_map;
  elmt_map = 0;
  
  if (num_elmts == 0) return "WARNING:  There are no elements!";
  
  elmt_map = new int[ num_elmts ];  SMART_ASSERT(elmt_map != 0);
  
  ex_opts(0);  // Temporarily turn off error reporting in case map isn't stored.
  int err = ex_get_elem_num_map(file_id, elmt_map);
  ex_opts(EX_VERBOSE);
  
  if (err < 0) {
    std::cout << "ExoII_Read::Load_Elmt_Map()  ERROR: Unable to load element map; "
         << "Exodus error = " << err << ".  Aborting..." << std::endl;
    exit(1);
  }
  else if (err > 0)
    return "WARNING: Default element map being used.";
  
  return "";
}

string ExoII_Read::Free_Elmt_Map()
{
  SMART_ASSERT(Check_State());
  
  delete [] elmt_map;
  elmt_map = 0;
  
  return "";
}

string ExoII_Read::Load_Elmt_Order()
{
  SMART_ASSERT(Check_State());
  
  if (!Open()) return "WARNING:  File not open!";
  
  delete [] elmt_order;
  elmt_order = 0;
  
  if (num_elmts == 0) return "WARNING:  There are no elements!";
  
  elmt_order = new int[ num_elmts ];  SMART_ASSERT(elmt_order != 0);
  
  ex_opts(0);  // Temporarily turn off error reporting in case map isn't stored.
  int err = ex_get_map(file_id, elmt_order);
  ex_opts(EX_VERBOSE);
  
  if (err < 0) {
    std::cout << "ExoII_Read::Load_Elmt_Map()  ERROR: Unable to load element order; "
         << "Exodus error = " << err << ".  Aborting..." << std::endl;
    exit(1);
  }
  else if (err > 0)
    return "WARNING: Default element order being used.";
  
  return "";
}

string ExoII_Read::Free_Elmt_Order()
{
  SMART_ASSERT(Check_State());
  
  delete [] elmt_order;
  elmt_order = 0;
  
  return "";
}

void ExoII_Read::Free_All_Maps()
{
  SMART_ASSERT(Check_State());
  
  delete [] node_map;      node_map = 0;
  delete [] elmt_map;      elmt_map = 0;
  delete [] elmt_order;  elmt_order = 0;
}


string ExoII_Read::Load_Nodal_Coordinates()
{
  SMART_ASSERT(Check_State());
  
  if (!Open()) return "WARNING:  File not open!";
  
  if (num_nodes)
  {
    nodes = new double[num_nodes * dimension];  SMART_ASSERT(nodes != 0);
    double *x = nodes, *y = nodes, *z = nodes;
    if (dimension > 1) y = nodes + num_nodes;
    if (dimension > 2) z = nodes + (2 * num_nodes);
    
    int err = ex_get_coord(file_id, x, y, z);
    if (err < 0) {
        std::cout << "ExoII_Read::Fill_Nodal_Coordinates(): ERROR: Failed to get "
             << "nodal coordinates!  Aborting..." << std::endl;
      exit(1);
    }
    else if (err > 0)
    {
      delete [] nodes;  nodes = 0;
      ostringstream oss;
      oss << "ExoII_Read::Fill_Nodal_Coordinates(): WARNING:  "
          << "Exodus issued warning \"" << err
          << "\" on call to ex_get_coord()!"
          << "  I'm not going to keep what it gave me for coordinates.";
      return oss.str();
    }
  }
  else
    return "WARNING:  There are no nodes!";
  
  return "";
}

void ExoII_Read::Free_Nodal_Coordinates()
{
  SMART_ASSERT(Check_State());
  delete [] nodes;  nodes = 0;
}

string ExoII_Read::Load_Nodal_Results(int time_step_num, int var_index)
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(time_step_num > 0 && time_step_num <= num_times);
  SMART_ASSERT(var_index >= 0 && (unsigned)var_index < nodal_vars.size());
  
  if (!Open()) return "WARNING:  File not open!";
  
  if (cur_time != time_step_num)
  {
    for (unsigned i = 0; i < nodal_vars.size(); ++i) {
      delete [] results[i];
      results[i] = 0;
    }
    cur_time = time_step_num;
  }
  
  if (num_nodes)
  {
    results[var_index] = new double[ num_nodes ];
    
    int err = ex_get_var(file_id,
			 cur_time,
			 EX_NODAL,
			 var_index+1,
			 0, num_nodes,
			 results[var_index]);
    if (err < 0) {
        std::cout << "ExoII_Read::Load_Nodal_Results(): ERROR: Failed to get "
             << "nodal variable values!  Aborting..." << std::endl;
      exit(1);
    }
    else if (err > 0)
    {
      delete [] results[var_index];  results[var_index] = 0;
      ostringstream oss;
      oss << "ExoII_Read::Load_Nodal_Results(): WARNING:  "
          << "Exodus issued warning \"" << err
          << "\" on call to ex_get_var()!"
          << "  I'm not going to keep what it gave me for values.";
      return oss.str();
    }
  }
  else
    return "WARNING:  There are no nodes!";
  
  return "";
}

void ExoII_Read::Free_Nodal_Results()
{
  SMART_ASSERT(Check_State());
  if (results)
    for (unsigned i = 0; i < nodal_vars.size(); ++i) {
      delete [] results[i];
      results[i] = 0;
    }
}

const double* ExoII_Read::Get_Nodal_Results(int var_index) const
{
  SMART_ASSERT(Check_State());
  if (cur_time == 0) return 0;
  SMART_ASSERT(var_index >= 0 && (unsigned)var_index < nodal_vars.size());
  
  return results[var_index];
}

string ExoII_Read::Load_Global_Results(int time_step_num)
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(time_step_num > 0 && time_step_num <= num_times);
  
  if (!Open()) return "WARNING:  File not open!";
  if (global_vars.size() == 0)
    return "WARNING:  No global variables! (doing nothing)";
  
  if (!global_vals) {
    global_vals = new double[global_vars.size()];
    SMART_ASSERT(global_vals != 0);
  }
    
  if (global_vals)
    for (unsigned j = 0; j < global_vars.size(); ++j)
      global_vals[j] = 0.0;
  
  int err = ex_get_glob_vars(file_id, time_step_num,
                             global_vars.size(), global_vals);
  
  if (err < 0) {
      std::cout << "ExoII_Read::Load_Global_Results(): ERROR: Failed to get "
           << "global variable values!  Aborting..." << std::endl;
    exit(1);
  }
  else if (err > 0) {
    ostringstream oss;
    oss << "ExoII_Read::Load_Global_Results(): WARNING:  "
        << "Exodus issued warning \"" << err
        << "\" on call to ex_get_glob_vars()!";
    return oss.str();
  }
  
  return "";
}

int ExoII_Read::Side_Set_Id(int set_index) const
{
  SMART_ASSERT(Check_State());
  
  if (set_index < 0 || set_index >= num_side_sets) return 0;
  
  return ssets[set_index].Id();
}

Side_Set* ExoII_Read::Get_Side_Set_by_Index(int side_set_index) const
{
  SMART_ASSERT(Check_State());
  
  if (side_set_index < 0 || side_set_index >= num_side_sets) return NULL;
  
  return &ssets[side_set_index];
}


Node_Set* ExoII_Read::Get_Node_Set_by_Index(int set_index) const
{
  SMART_ASSERT(Check_State());
  
  if (set_index < 0 || set_index >= num_node_sets) return NULL;
  
  return &nsets[set_index];
}


// **********************  Misc functions  *************************** //

// This function converts an Exodus global element number (1-offset) into
// its block index (0-offset) and block element index (0-offset).
string ExoII_Read::Global_to_Block_Local(int global_elmt_num,
                                          int& block_index,
                                          int& local_elmt_index) const
{
  SMART_ASSERT(Check_State());
  
  if (!Open()) return "ERROR:  File not open!";
  if (global_elmt_num < 1 || global_elmt_num > num_elmts) {
    ostringstream oss;
    oss << "ERROR:  global_elmt_num = " << global_elmt_num
        << " is out of bounds [1, " << num_elmts << "]!";
    return oss.str();
  }
  
  block_index = 0;
  
  int total = 0;
  while (total + eblocks[block_index].Size() < global_elmt_num)
    total += eblocks[block_index++].Size();
  
  local_elmt_index = global_elmt_num - total - 1;
  
  return "";
}

void ExoII_Read::Display_Stats(std::ostream& s) const
{
  SMART_ASSERT(Check_State());
  
  const char* separator = "       --------------------------------------------------";
  
  s << "ExoII_Read::Display():  file name = " << file_name << std::endl;
  
  if (title != "")
    s << "                             title = " << title << std::endl;
  
  s << "                           file id = ";
  if (file_id >= 0) s << file_id << std::endl;
  else              s << "(not open)" << std::endl;
  
  s << "                         dimension = " << dimension << std::endl
    << "                   number of nodes = " << num_nodes << std::endl
    << "                number of elements = " << num_elmts << std::endl;
  
  if (dimension >= 1) {
    if (coord_names.size() >= 1)
      s << "             first coordinate name = " << coord_names[0] << std::endl;
    else
      s << "             first coordinate name = " << std::endl;
  }
  if (dimension >= 2) {
    if (coord_names.size() >= 2)
      s << "            second coordinate name = " << coord_names[1] << std::endl;
    else
      s << "            second coordinate name = " << std::endl;
  }
  if (dimension >= 3) {
    if (coord_names.size() >= 3)
      s << "             third coordinate name = " << coord_names[2] << std::endl;
    else
      s << "             third coordinate name = " << std::endl;
  }
  
  s << separator << std::endl;
  s << "          number of element blocks = " << num_elmt_blocks << std::endl
    << "              number of nodes sets = " << num_node_sets << std::endl
    << "               number of side sets = " << num_side_sets << std::endl;
  
  if (num_elmt_blocks)
  {
    s << separator << std::endl;
    
    s << "                   ELEMENT BLOCKS" << std::endl;
    s << "\tIndex \tId     num elmts    nodes/elmt num attr  type" << std::endl;
    for (int b = 0; b < num_elmt_blocks; ++b)
    {
      s << "\t" << b << "   \t" << eblocks[b].Id()
        << "  \t"    << eblocks[b].Size()
        << "  \t\t"  << eblocks[b].num_nodes_per_elmt
        << "  \t  "  << eblocks[b].attr_count()
        << "  \t "   << eblocks[b].elmt_type << std::endl;
    }
  }
  
  if (num_node_sets)
  {
    s << separator << std::endl;
    
    s << "              NODE SETS " << std::endl
      << "\tIndex \tId     length \tdistribution factors length" << std::endl;
    for (int nset = 0; nset < num_node_sets; ++nset)
    {
      s << "\t"   << nset << "  \t" << nsets[nset].Id()
        << "  \t" << nsets[nset].Size()
        << "  \t" << nsets[nset].num_dist_factors << std::endl;
    }
  }
  
  if (num_side_sets)
  {
    s << separator << std::endl;
    
    s << "              SIDE SETS " << std::endl
      << "\tIndex \tId     length \tdistribution factors length" << std::endl;
    for (int sset = 0; sset < num_side_sets; ++sset)
    {
      s << "\t"   << sset << "  \t" << ssets[sset].Id()
        << "  \t" << ssets[sset].Size()
        << "  \t" << ssets[sset].num_dist_factors << std::endl;
    }
  }
  
  if (io_word_size || db_version > 0.0 || api_version > 0.0)
    s << separator << std::endl;
  if (io_word_size)
    s << "                  file's data size = " << io_word_size
      << " bytes" << std::endl;
  if (db_version > 0.0)
    s << "           Exodus database version = " << db_version << std::endl;
  if (api_version > 0.0)
    s << "            Exodus library version = " << api_version << std::endl;
  
  s << separator << std::endl;
  
  if (num_times)  // Use this to indicate whether results data exists.
  {
    s << "\t\tRESULTS INFO" << std::endl << separator << std::endl;
    
    s << "           number global variables = " << global_vars.size() << std::endl
      << "            number nodal variables = " << nodal_vars.size()  << std::endl
      << "          number element variables = " << elmt_vars.size()   << std::endl;
    
    unsigned max = global_vars.size() > nodal_vars.size() ?
                   global_vars.size(): nodal_vars.size();
    max = elmt_vars.size() > max ? elmt_vars.size(): max;
    
    if (max) s << "\t  GLOBAL    \t  NODAL    \t  ELEMENT" << std::endl;
    for (unsigned i = 0; i < max; ++i)
    {
      if (i < global_vars.size()) s << "\t    " << global_vars[i];
      else s << "\t          ";
      if (i < nodal_vars.size()) s << "\t    " << nodal_vars[i];
      else s << "\t          ";
      if (i < elmt_vars.size()) s << "\t    " << elmt_vars[i] << std::endl;
      else s << std::endl;
    }
    
    s << separator << std::endl;
    
    s << "                   number of times = " << num_times << std::endl;
    for (int t = 0; t < num_times; ++t)
      s << "\t\t(" << (t+1) << ") " << times[t] << std::endl;
    
  }
}

void ExoII_Read::Display(std::ostream& s) const
{
  SMART_ASSERT(Check_State());
  
  const char* separator = "       --------------------------------------------------";
  
  s << "ExoII_Read::Display():  file name = " << file_name << std::endl;
  
  if (title != "")
    s << "                             title = " << title << std::endl;
  
  s << "                           file id = ";
  if (file_id >= 0) s << file_id << std::endl;
  else              s << "(not open)" << std::endl;
  
  s << "                         dimension = " << dimension << std::endl
    << "                   number of nodes = " << num_nodes << std::endl
    << "                number of elements = " << num_elmts << std::endl;
  
  if (dimension >= 1) {
    if (coord_names.size() >= 1)
      s << "             first coordinate name = " << coord_names[0] << std::endl;
    else
      s << "             first coordinate name = " << std::endl;
  }
  if (dimension >= 2) {
    if (coord_names.size() >= 2)
      s << "            second coordinate name = " << coord_names[1] << std::endl;
    else
      s << "            second coordinate name = " << std::endl;
  }
  if (dimension >= 3) {
    if (coord_names.size() >= 3)
      s << "             third coordinate name = " << coord_names[2] << std::endl;
    else
      s << "             third coordinate name = " << std::endl;
  }
  
  s << separator << std::endl;
  s << "          number of element blocks = " << num_elmt_blocks << std::endl
    << "              number of nodes sets = " << num_node_sets << std::endl
    << "               number of side sets = " << num_side_sets << std::endl;
  
  if (num_elmt_blocks)
  {
    s << separator << std::endl;
    
    s << "                   ELEMENT BLOCKS" << std::endl;
    s << "\tIndex \tId     num elmts    nodes/elmt num attr  type" << std::endl;
    for (int b = 0; b < num_elmt_blocks; ++b)
    {
      s << "\t" << b << "   \t" << eblocks[b].Id()
        << "  \t"    << eblocks[b].Size()
        << "  \t\t"  << eblocks[b].num_nodes_per_elmt
        << "  \t  "  << eblocks[b].attr_count()
        << "  \t "   << eblocks[b].elmt_type << std::endl;
    }
  }
  
  if (num_node_sets)
  {
    s << separator << std::endl;
    
    s << "              NODE SETS " << std::endl
      << "\tIndex \tId     length \tdistribution factors length" << std::endl;
    for (int nset = 0; nset < num_node_sets; ++nset)
    {
      s << "\t"   << nset << "  \t" << nsets[nset].Id()
        << "  \t" << nsets[nset].Size()
        << "  \t" << nsets[nset].num_dist_factors << std::endl;
    }
  }
  
  if (num_side_sets)
  {
    s << separator << std::endl;
    
    s << "              SIDE SETS " << std::endl
      << "\tIndex \tId     length \tdistribution factors length" << std::endl;
    for (int sset = 0; sset < num_side_sets; ++sset)
    {
      s << "\t"   << sset << "  \t" << ssets[sset].Id()
        << "  \t" << ssets[sset].Size()
        << "  \t" << ssets[sset].num_dist_factors << std::endl;
    }
  }
  
  if (io_word_size || db_version > 0.0 || api_version > 0.0)
    s << separator << std::endl;
  if (io_word_size)
    s << "                  file's data size = " << io_word_size
      << " bytes" << std::endl;
  if (db_version > 0.0)
    s << "           Exodus database version = " << db_version << std::endl;
  if (api_version > 0.0)
    s << "            Exodus library version = " << api_version << std::endl;
  
  s << separator << std::endl;
  
  if (nodes)
  {
    s << "\tNodal Coordinates:" << std::endl;
    for (int n = 0; n < num_nodes; ++n)
    {
      s << "\t" << (n+1) << "\t" << nodes[n];
      if (dimension > 1) s << "\t" << nodes[ num_nodes + n ];
      if (dimension > 2) s << "\t" << nodes[ 2 * num_nodes + n ];
      s << std::endl;
    }
    s << separator << std::endl;
  }
  
  if (!elmt_atts.empty()) {
    s << "         number element attributes = " << elmt_atts.size()   << std::endl;
  }

  if (num_times)  // Use this to indicate whether results data exists.
  {
    s << "\t\tRESULTS INFO" << std::endl << separator << std::endl;
    
    s << "           number global variables = " << global_vars.size() << std::endl
      << "            number nodal variables = " << nodal_vars.size()  << std::endl
      << "          number element variables = " << elmt_vars.size()   << std::endl;
    
    unsigned max = global_vars.size() > nodal_vars.size() ?
                   global_vars.size(): nodal_vars.size();
    max = elmt_vars.size() > max ? elmt_vars.size(): max;
    
    if (max) s << "\t  GLOBAL    \t  NODAL    \t  ELEMENT" << std::endl;
    for (unsigned i = 0; i < max; ++i)
    {
      if (i < global_vars.size()) s << "\t    " << global_vars[i];
      else s << "\t          ";
      if (i < nodal_vars.size()) s << "\t    " << nodal_vars[i];
      else s << "\t          ";
      if (i < elmt_vars.size()) s << "\t    " << elmt_vars[i] << std::endl;
      else s << std::endl;
    }
    
    s << separator << std::endl;
    
    s << "                   number of times = " << num_times << std::endl;
    for (int t = 0; t < num_times; ++t)
      s << "\t\t(" << (t+1) << ") " << times[t] << std::endl;
    
  }
}

void ExoII_Read::Display_Maps(std::ostream& s) const
{
  SMART_ASSERT(Check_State());
  
  s << "ExoII_Read::Display_Maps()  file name = " << file_name << std::endl;
  if (node_map == 0)
    s << "                       (node map is not loaded)" << std::endl;
  if (elmt_map == 0)
    s << "                       (elmt map is not loaded)" << std::endl;
  
  if (node_map != 0 || elmt_map != 0)
  {
    s << "\tNode Map\tElmt Map" << std::endl;
    
    int max = num_nodes > num_elmts ? num_nodes : num_elmts;
    for (int i = 0; i < max; ++i)
    {
      s << "   " << (i+1) << ")\t";
      if (i < num_nodes && node_map != 0) s << node_map[i];
      s << "\t";
      if (i < num_elmts && elmt_map != 0) s << elmt_map[i];
      s << std::endl;
    }
  }
}

int ExoII_Read::Elmt_Block_Index(int eblock_id) const
{
  SMART_ASSERT(Check_State());
  for (int b = 0; b < num_elmt_blocks; ++b)
    if (eblocks[b].Id() == eblock_id)
      return b;
  return -1;
}

int ExoII_Read::NSet_Index(int nset_id) const
{
  SMART_ASSERT(Check_State());
  for (int n = 0; n < num_node_sets; ++n)
    if (nsets[n].Id() == nset_id)
      return n;
  return -1;
}

int ExoII_Read::SSet_Index(int sset_id) const
{
  SMART_ASSERT(Check_State());
  for (int s = 0; s < num_side_sets; ++s)
    if (ssets[s].Id() == sset_id)
      return s;
  return -1;
}

int ExoII_Read::Check_State() const
{
  SMART_ASSERT(file_id         >= -1);
  SMART_ASSERT(num_nodes       >= 0);
  SMART_ASSERT(dimension       >= 0);
  SMART_ASSERT(num_elmts       >= 0);
  SMART_ASSERT(num_elmt_blocks >= 0);
  SMART_ASSERT(num_node_sets   >= 0);
  SMART_ASSERT(num_side_sets   >= 0);
  SMART_ASSERT(db_version      >= 0.0);
  SMART_ASSERT(api_version     >= 0.0);
  SMART_ASSERT(io_word_size == 0 || io_word_size == 4 || io_word_size == 8);
  
  SMART_ASSERT( !( file_id >= 0 && io_word_size == 0) );
  SMART_ASSERT( !( file_id >= 0 && file_name == "") );
  
  SMART_ASSERT( !( num_elmt_blocks > 0 && !eblocks ) );
  SMART_ASSERT( !( num_node_sets   > 0 && !nsets ) );
  SMART_ASSERT( !( num_side_sets   > 0 && !ssets ) );
  
  SMART_ASSERT( !( num_nodes == 0 && nodes ) );
  
  SMART_ASSERT(num_times >= 0);
  SMART_ASSERT( !( num_times > 0 && !times ) );
  
  SMART_ASSERT(cur_time >= 0 && cur_time <= num_times);
  SMART_ASSERT( !( nodal_vars.size() > 0 && !results ) );
  SMART_ASSERT( !( nodal_vars.size() == 0 && results ) );

// #ifndef NDEBUG
//   int e = 0;
//   for (int b = 0; b < num_elmt_blocks; ++b)
//     e += eblocks[b].Num_Elmts();
//   SMART_ASSERT(e == num_elmts);
// #endif
  
  return 1;
}
