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
#include "exoII_read.h"
#include "exo_block.h"        // for Exo_Block
#include "libmesh/exodusII.h" // for ex_init_params, ex_opts, etc
#include "node_set.h"         // for Node_Set
#include "smart_assert.h"     // for SMART_ASSERT, Assert, etc
#include "stringx.h"          // for chop_whitespace
#include "terminal_color.h"   // for operator<<, normal, red
#include "util.h"             // for free_name_array, etc
#include <algorithm>          // for copy
#include <cstdint>            // for int64_t
#include <cstdio>             // for fclose, FILE, fopen
#include <cstdlib>            // for exit
#include <cstring>            // for strlen
#include <iostream>
#include <iostream> // for operator<<, basic_ostream, etc
#include <set>      // for set
#include <string>   // for string, char_traits, etc
#include <vector>   // for vector

namespace
{
void read_vars(
    int file_id, EXOTYPE flag, const char * type, int num_vars, std::vector<std::string> & varlist);
} // namespace

template <typename INT>
ExoII_Read<INT>::ExoII_Read()
  : file_id(-1), // value of -1 indicates file not open
    num_nodes(0),
    dimension(0),
    num_elmts(0),
    num_elmt_blocks(0),
    num_node_sets(0),
    num_side_sets(0),
    db_version(0.0),
    api_version(0.0),
    io_word_size(0),
    eblocks(nullptr),
    nsets(nullptr),
    ssets(nullptr),
    nodes(nullptr),
    node_map(nullptr),
    elmt_map(nullptr),
    elmt_order(nullptr),
    num_times(0),
    times(nullptr),
    cur_time(0),
    results(nullptr),
    global_vals(nullptr),
    global_vals2(nullptr)
{
}

template <typename INT>
ExoII_Read<INT>::ExoII_Read(const char * fname)
  : file_name(fname),
    file_id(-1), // value of -1 indicates file not open
    num_nodes(0),
    dimension(0),
    num_elmts(0),
    num_elmt_blocks(0),
    num_node_sets(0),
    num_side_sets(0),
    db_version(0.0),
    api_version(0.0),
    io_word_size(0),
    eblocks(nullptr),
    nsets(nullptr),
    ssets(nullptr),
    nodes(nullptr),
    node_map(nullptr),
    elmt_map(nullptr),
    elmt_order(nullptr),
    num_times(0),
    times(nullptr),
    cur_time(0),
    results(nullptr),
    global_vals(nullptr),
    global_vals2(nullptr)
{
}

template <typename INT>
ExoII_Read<INT>::~ExoII_Read()
{
  try
  {
    SMART_ASSERT(Check_State());

    if (file_id >= 0)
    {
      std::string err = Close_File();
      if (!err.empty())
      {
        ERROR("ExoII_Read destructor(): closing file:"
              << " \"" << err << "\"\n");
      }
    }

    delete[] eblocks;
    delete[] nsets;
    delete[] ssets;
    delete[] nodes;
    delete[] times;
    if (results)
    {
      for (unsigned i = 0; i < nodal_vars.size(); ++i)
      {
        delete[] results[i];
      }
      delete[] results;
    }
    delete[] global_vals;
    delete[] global_vals2;
    delete[] node_map;
    delete[] elmt_map;
    delete[] elmt_order;
  }
  catch (...)
  {
  }
}

template <typename INT>
std::string
ExoII_Read<INT>::Close_File()
{
  SMART_ASSERT(Check_State());

  if (file_id < 0)
  {
    return "exodiff: ERROR: File is not open!";
  }
  int err = ex_close(file_id);

  if (err < 0)
  {
    ERROR("ExoII_Read::Close_File(): " << err << ": Unable to close file!  Aborting...\n");
    exit(1);
  }
  if (err > 0)
  {
    std::ostringstream oss;
    oss << "WARNING: " << err << " issued upon close";
    return oss.str();
  }

  file_id = -1;

  return "";
}

template <typename INT>
double
ExoII_Read<INT>::Time(int time_num) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(time_num > 0 && time_num <= num_times)(time_num)(num_times);
  return times[time_num - 1];
}

template <typename INT>
const std::string &
ExoII_Read<INT>::Global_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < global_vars.size());
  return global_vars[index];
}

template <typename INT>
const std::string &
ExoII_Read<INT>::Nodal_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < nodal_vars.size());
  return nodal_vars[index];
}

template <typename INT>
const std::string &
ExoII_Read<INT>::Elmt_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < elmt_vars.size());
  return elmt_vars[index];
}

template <typename INT>
const std::string &
ExoII_Read<INT>::Elmt_Att_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < elmt_atts.size());
  return elmt_atts[index];
}

template <typename INT>
const std::string &
ExoII_Read<INT>::NS_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < ns_vars.size());
  return ns_vars[index];
}

template <typename INT>
const std::string &
ExoII_Read<INT>::SS_Var_Name(int index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(index >= 0 && (unsigned)index < ss_vars.size());
  return ss_vars[index];
}

template <typename INT>
Exo_Block<INT> *
ExoII_Read<INT>::Get_Elmt_Block_by_Index(size_t block_index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(block_index < num_elmt_blocks);
  return &eblocks[block_index];
}

template <typename INT>
Exo_Block<INT> *
ExoII_Read<INT>::Get_Elmt_Block_by_Id(size_t id) const
{
  SMART_ASSERT(Check_State());
  for (size_t i = 0; i < num_elmt_blocks; i++)
  {
    if (eblocks[i].Id() == id)
    {
      return &eblocks[i];
    }
  }
  return nullptr;
}

template <typename INT>
Exo_Block<INT> *
ExoII_Read<INT>::Get_Elmt_Block_by_Name(const std::string & name) const
{
  SMART_ASSERT(Check_State());
  for (size_t i = 0; i < num_elmt_blocks; i++)
  {
    if (eblocks[i].Name() == name)
    {
      return &eblocks[i];
    }
  }
  return nullptr;
}

template <typename INT>
Exo_Entity *
ExoII_Read<INT>::Get_Entity_by_Index(EXOTYPE type, size_t block_index) const
{
  SMART_ASSERT(Check_State());

  switch (type)
  {
    case EX_ELEM_BLOCK:
      SMART_ASSERT(block_index < num_elmt_blocks);
      return &eblocks[block_index];
    case EX_NODE_SET:
      SMART_ASSERT(block_index < num_node_sets);
      return &nsets[block_index];
    case EX_SIDE_SET:
      SMART_ASSERT(block_index < num_side_sets);
      return &ssets[block_index];
    default:
      return nullptr;
  }
}

template <typename INT>
Exo_Entity *
ExoII_Read<INT>::Get_Entity_by_Id(EXOTYPE type, size_t id) const
{
  SMART_ASSERT(Check_State());
  switch (type)
  {
    case EX_ELEM_BLOCK:
      for (size_t i = 0; i < num_elmt_blocks; i++)
      {
        if (eblocks[i].Id() == id)
        {
          return &eblocks[i];
        }
      }
      break;
    case EX_NODE_SET:
      for (size_t i = 0; i < num_node_sets; i++)
      {
        if (nsets[i].Id() == id)
        {
          return &nsets[i];
        }
      }
      break;
    case EX_SIDE_SET:
      for (size_t i = 0; i < num_side_sets; i++)
      {
        if (ssets[i].Id() == id)
        {
          return &ssets[i];
        }
      }
      break;
    default:
      return nullptr;
  }
  return nullptr;
}

template <typename INT>
Exo_Entity *
ExoII_Read<INT>::Get_Entity_by_Name(EXOTYPE type, const std::string & name) const
{
  SMART_ASSERT(Check_State());
  switch (type)
  {
    case EX_ELEM_BLOCK:
      for (size_t i = 0; i < num_elmt_blocks; i++)
      {
        if (eblocks[i].Name() == name)
        {
          return &eblocks[i];
        }
      }
      break;
    case EX_NODE_SET:
      for (size_t i = 0; i < num_node_sets; i++)
      {
        if (nsets[i].Name() == name)
        {
          return &nsets[i];
        }
      }
      break;
    case EX_SIDE_SET:
      for (size_t i = 0; i < num_side_sets; i++)
      {
        if (ssets[i].Name() == name)
        {
          return &ssets[i];
        }
      }
      break;
    default:
      return nullptr;
  }
  return nullptr;
}

template <typename INT>
Node_Set<INT> *
ExoII_Read<INT>::Get_Node_Set_by_Id(size_t set_id) const
{
  SMART_ASSERT(Check_State());
  for (size_t i = 0; i < num_node_sets; i++)
  {
    if (nsets[i].Id() == set_id)
    {
      return &nsets[i];
    }
  }
  return nullptr;
}

template <typename INT>
Node_Set<INT> *
ExoII_Read<INT>::Get_Node_Set_by_Name(const std::string & name) const
{
  SMART_ASSERT(Check_State());
  for (size_t i = 0; i < num_node_sets; i++)
  {
    if (nsets[i].Name() == name)
    {
      return &nsets[i];
    }
  }
  return nullptr;
}

template <typename INT>
Side_Set<INT> *
ExoII_Read<INT>::Get_Side_Set_by_Id(size_t set_id) const
{
  SMART_ASSERT(Check_State());
  for (size_t i = 0; i < num_side_sets; i++)
  {
    if (ssets[i].Id() == set_id)
    {
      return &ssets[i];
    }
  }
  return nullptr;
}

template <typename INT>
Side_Set<INT> *
ExoII_Read<INT>::Get_Side_Set_by_Name(const std::string & name) const
{
  SMART_ASSERT(Check_State());
  for (size_t i = 0; i < num_side_sets; i++)
  {
    if (ssets[i].Name() == name)
    {
      return &ssets[i];
    }
  }
  return nullptr;
}

template <typename INT>
std::string
ExoII_Read<INT>::Load_Elmt_Block_Description(size_t block_index) const
{
  SMART_ASSERT(Check_State());
  if (!Open())
  {
    return "exodiff: ERROR:  Must open file before loading blocks!";
  }
  SMART_ASSERT(block_index < num_elmt_blocks);

  eblocks[block_index].Load_Connectivity();
  //  eblocks[idx].Load_Connectivity();
  //  eblocks[idx].Load_Attributes();

  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Load_Elmt_Block_Descriptions() const
{
  SMART_ASSERT(Check_State());
  if (!Open())
  {
    return "exodiff: ERROR:  Must open file before loading blocks!";
  }
  for (size_t b = 0; b < num_elmt_blocks; ++b)
  {
    eblocks[b].Load_Connectivity();
    //    eblocks[b].Load_Attributes();
  }

  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Free_Elmt_Block(size_t block_index) const
{
  SMART_ASSERT(Check_State());

  SMART_ASSERT(block_index < num_elmt_blocks);

  eblocks[block_index].Free_Connectivity();
  eblocks[block_index].Free_Attributes();
  //  eblocks[idx].Free_Connectivity();
  //  eblocks[idx].Free_Attributes();

  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Free_Elmt_Blocks() const
{
  SMART_ASSERT(Check_State());

  for (size_t b = 0; b < num_elmt_blocks; ++b)
  {
    eblocks[b].Free_Connectivity();
    eblocks[b].Free_Attributes();
  }

  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Give_Connectivity(size_t block_index,
                                   size_t & num_e,
                                   size_t & npe,
                                   INT *& new_conn)
{
  SMART_ASSERT(block_index < num_elmt_blocks);

  return eblocks[block_index].Give_Connectivity(num_e, npe, new_conn);
}

template <typename INT>
size_t
ExoII_Read<INT>::Block_Id(size_t block_index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(block_index < num_elmt_blocks);
  return eblocks[block_index].Id();
}

template <typename INT>
std::string
ExoII_Read<INT>::Load_Node_Map()
{
  SMART_ASSERT(Check_State());

  if (!Open())
  {
    return "WARNING:  File not open!";
  }
  delete[] node_map;
  node_map = nullptr;

  if (num_nodes == 0)
  {
    return "WARNING:  There are no nodes!";
  }
  node_map = new INT[num_nodes];
  SMART_ASSERT(node_map != nullptr);

  ex_opts(0); // Temporarily turn off error reporting in case map isn't stored.
  int err = ex_get_id_map(file_id, EX_NODE_MAP, node_map);
  ex_opts(EX_VERBOSE);

  if (err < 0)
  {
    ERROR("Unable to load node map; "
          << "Exodus error = " << err << ".  Aborting...\n");
    exit(1);
  }
  else if (err > 0)
  {
    return "WARNING: Default node map being used.";
  }
  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Free_Node_Map()
{
  SMART_ASSERT(Check_State());

  delete[] node_map;
  node_map = nullptr;

  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Load_Elmt_Map()
{
  SMART_ASSERT(Check_State());

  if (!Open())
  {
    return "WARNING:  File not open!";
  }
  delete[] elmt_map;
  elmt_map = nullptr;

  if (num_elmts == 0)
  {
    return "WARNING:  There are no elements!";
  }
  elmt_map = new INT[num_elmts];
  SMART_ASSERT(elmt_map != nullptr);

  ex_opts(0); // Temporarily turn off error reporting in case map isn't stored.
  int err = ex_get_id_map(file_id, EX_ELEM_MAP, elmt_map);
  ex_opts(EX_VERBOSE);

  if (err < 0)
  {
    ERROR("Unable to load element map; "
          << "Exodus error = " << err << ".  Aborting...\n");
    exit(1);
  }
  else if (err > 0)
  {
    return "WARNING: Default element map being used.";
  }
  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Free_Elmt_Map()
{
  SMART_ASSERT(Check_State());

  delete[] elmt_map;
  elmt_map = nullptr;

  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Load_Nodal_Coordinates()
{
  SMART_ASSERT(Check_State());

  if (!Open())
  {
    return "WARNING:  File not open!";
  }
  if (num_nodes)
  {
    size_t count = num_nodes * dimension;
    nodes = new double[count];
    SMART_ASSERT(nodes != nullptr);
    double *x = nodes, *y = nodes, *z = nodes;
    if (dimension > 1)
    {
      y = nodes + num_nodes;
    }
    if (dimension > 2)
    {
      z = nodes + (2 * num_nodes);
    }

    int err = ex_get_coord(file_id, x, y, z);
    if (err < 0)
    {
      ERROR("Failed to get "
            << "nodal coordinates!  Aborting...\n");
      exit(1);
    }
    else if (err > 0)
    {
      delete[] nodes;
      nodes = nullptr;
      std::ostringstream oss;
      oss << "exodiff: WARNING:  "
          << "Exodus issued warning \"" << err << "\" on call to ex_get_coord()!"
          << "  I'm not going to keep what it gave me for coordinates.";
      return oss.str();
    }
  }
  else
  {
    return "WARNING:  There are no nodes!";
  }
  return "";
}

template <typename INT>
void
ExoII_Read<INT>::Free_Nodal_Coordinates()
{
  SMART_ASSERT(Check_State());
  delete[] nodes;
  nodes = nullptr;
}

template <typename INT>
std::string
ExoII_Read<INT>::Load_Nodal_Results(int time_step_num, int var_index)
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(time_step_num > 0 && time_step_num <= num_times);
  SMART_ASSERT(var_index >= 0 && (unsigned)var_index < nodal_vars.size());

  if (!Open())
  {
    return "WARNING:  File not open!";
  }
  if (cur_time != time_step_num)
  {
    for (unsigned i = 0; i < nodal_vars.size(); ++i)
    {
      delete[] results[i];
      results[i] = nullptr;
    }
    cur_time = time_step_num;
  }

  if (num_nodes)
  {
    results[var_index] = new double[num_nodes];

    int err =
        ex_get_var(file_id, cur_time, EX_NODAL, var_index + 1, 0, num_nodes, results[var_index]);
    if (err < 0)
    {
      ERROR("ExoII_Read::Load_Nodal_Results(): Failed to get "
            << "nodal variable values!  Aborting...\n");
      exit(1);
    }
    else if (err > 0)
    {
      delete[] results[var_index];
      results[var_index] = nullptr;
      std::ostringstream oss;
      oss << "ExoII_Read::Load_Nodal_Results(): WARNING:  "
          << "Exodus issued warning \"" << err << "\" on call to ex_get_var()!"
          << "  I'm not going to keep what it gave me for values.";
      return oss.str();
    }
  }
  else
  {
    return "WARNING:  There are no nodes!";
  }
  return "";
}

template <typename INT>
const double * ExoII_Read<INT>::Get_Nodal_Results(int t1,
                                                  int t2,
                                                  double proportion,
                                                  int var_index) const // Interpolated results.
{
  static double * st_results = nullptr;
  static double * st_results2 = nullptr;

  SMART_ASSERT(Check_State());
  SMART_ASSERT(t1 > 0 && t1 <= num_times);
  SMART_ASSERT(t2 > 0 && t2 <= num_times);
  SMART_ASSERT(var_index >= 0 && (unsigned)var_index < nodal_vars.size());

  if (!Open())
  {
    return nullptr;
  }

  if (st_results == nullptr)
  {
    st_results = new double[num_nodes];
  }

  int err = ex_get_var(file_id, t1, EX_NODAL, var_index + 1, 0, num_nodes, st_results);
  if (err < 0)
  {
    ERROR("ExoII_Read::Get_Nodal_Results(): Failed to get "
          << "nodal variable values!  Aborting...\n");
    exit(1);
  }

  if (t1 != t2)
  {
    if (st_results2 == nullptr)
    {
      st_results2 = new double[num_nodes];
    }

    err = ex_get_var(file_id, t2, EX_NODAL, var_index + 1, 0, num_nodes, st_results2);
    if (err < 0)
    {
      ERROR("ExoII_Read::Load_Nodal_Results(): Failed to get "
            << "nodal variable values!  Aborting...\n");
      exit(1);
    }

    // Interpolate the values...
    for (size_t i = 0; i < num_nodes; i++)
    {
      st_results[i] = (1.0 - proportion) * st_results[i] + proportion * st_results2[i];
    }
  }
  return st_results;
}

template <typename INT>
void
ExoII_Read<INT>::Free_Nodal_Results()
{
  SMART_ASSERT(Check_State());
  if (results)
  {
    for (unsigned i = 0; i < nodal_vars.size(); ++i)
    {
      delete[] results[i];
      results[i] = nullptr;
    }
  }
}

template <typename INT>
void
ExoII_Read<INT>::Free_Nodal_Results(int var_index)
{
  SMART_ASSERT(Check_State());
  if (results)
  {
    if (results[var_index])
    {
      delete[] results[var_index];
      results[var_index] = nullptr;
    }
  }
}

template <typename INT>
const double *
ExoII_Read<INT>::Get_Nodal_Results(int var_index) const
{
  SMART_ASSERT(Check_State());
  if (cur_time == 0)
  {
    return nullptr;
  }
  SMART_ASSERT(var_index >= 0 && (unsigned)var_index < nodal_vars.size());

  return results[var_index];
}

template <typename INT>
std::string
ExoII_Read<INT>::Load_Global_Results(int time_step_num)
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(time_step_num > 0 && time_step_num <= num_times);

  if (!Open())
  {
    return "WARNING:  File not open!";
  }
  if (global_vars.empty())
  {
    return "WARNING:  No global variables! (doing nothing)";
  }

  if (global_vals == nullptr)
  {
    global_vals = new double[global_vars.size()];
    SMART_ASSERT(global_vals != nullptr);
  }

  if (global_vals)
  {
    for (unsigned j = 0; j < global_vars.size(); ++j)
    {
      global_vals[j] = 0.0;
    }

    int err = ex_get_var(file_id, time_step_num, EX_GLOBAL, 1, 1, global_vars.size(), global_vals);

    if (err < 0)
    {
      ERROR("ExoII_Read::Load_Global_Results(): Failed to get "
            << "global variable values!  Aborting...\n");
      exit(1);
    }
    else if (err > 0)
    {
      std::ostringstream oss;
      oss << "ExoII_Read::Load_Global_Results(): WARNING:  "
          << "Exodus issued warning \"" << err << "\" on call to ex_get_glob_vars()!";
      return oss.str();
    }
  }
  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Load_Global_Results(int t1, int t2, double proportion)
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(t1 > 0 && t1 <= num_times);
  SMART_ASSERT(t2 > 0 && t2 <= num_times);

  if (!Open())
  {
    return "WARNING:  File not open!";
  }
  if (global_vars.empty())
  {
    return "WARNING:  No global variables! (doing nothing)";
  }
  if (global_vals == nullptr)
  {
    global_vals = new double[global_vars.size()];
    SMART_ASSERT(global_vals != nullptr);
  }

  if (t2 != t1 && (global_vals2 == nullptr))
  {
    global_vals2 = new double[global_vars.size()];
    SMART_ASSERT(global_vals2 != nullptr);
  }

  if (global_vals)
  {
    for (unsigned j = 0; j < global_vars.size(); ++j)
    {
      global_vals[j] = 0.0;
    }

    int err = ex_get_var(file_id, t1, EX_GLOBAL, 1, 1, global_vars.size(), global_vals);

    if (err < 0)
    {
      ERROR("ExoII_Read::Load_Global_Results(): Failed to get "
            << "global variable values!  Aborting...\n");
      exit(1);
    }

    if (t2 != t1)
    {
      err = ex_get_var(file_id, t2, EX_GLOBAL, 1, 1, global_vars.size(), global_vals2);
      if (err < 0)
      {
        ERROR("ExoII_Read::Load_Global_Results(): Failed to get "
              << "global variable values!  Aborting...\n");
        exit(1);
      }

      // Do the interpolation...
      for (size_t j = 0; j < global_vars.size(); j++)
      {
        global_vals[j] = (1.0 - proportion) * global_vals[j] + proportion * global_vals2[j];
      }
    }
  }
  return "";
}

template <typename INT>
Side_Set<INT> *
ExoII_Read<INT>::Get_Side_Set_by_Index(size_t side_set_index) const
{
  SMART_ASSERT(Check_State());

  if (side_set_index >= num_side_sets)
  {
    return nullptr;
  }

  return &ssets[side_set_index];
}

template <typename INT>
Node_Set<INT> *
ExoII_Read<INT>::Get_Node_Set_by_Index(size_t set_index) const
{
  SMART_ASSERT(Check_State());

  if (set_index >= num_node_sets)
  {
    return nullptr;
  }

  return &nsets[set_index];
}

// **********************  Misc functions  *************************** //

// This function converts an Exodus global element number (1-offset) into
// its block index (0-offset) and block element index (0-offset).
template <typename INT>
std::string
ExoII_Read<INT>::Global_to_Block_Local(size_t global_elmt_num,
                                       int & block_index,
                                       size_t & local_elmt_index) const
{
  SMART_ASSERT(Check_State());

  if (!Open())
  {
    return "exodiff: ERROR:  File not open!";
  }
  if (global_elmt_num < 1 || global_elmt_num > num_elmts)
  {
    std::ostringstream oss;
    oss << "exodiff: ERROR:  global_elmt_num = " << global_elmt_num << " is out of bounds [1, "
        << num_elmts << "]!";
    return oss.str();
  }

  block_index = 0;

  size_t total = 0;
  while (total + eblocks[block_index].Size() < global_elmt_num)
  {
    total += eblocks[block_index++].Size();
  }

  local_elmt_index = global_elmt_num - total - 1;

  return "";
}

template <typename INT>
void
ExoII_Read<INT>::Display(std::ostream & s) const
{
  SMART_ASSERT(Check_State());

  const char * separator = "       --------------------------------------------------";

  s << "ExoII_Read::Display():  file name = " << file_name << '\n';

  if (title != "")
  {
    s << "                             title = " << title << '\n';
  }
  s << "                           file id = ";
  if (file_id >= 0)
  {
    s << file_id << '\n';
  }
  else
  {
    s << "(not open)\n";
  }
  s << "                         dimension = " << dimension << '\n'
    << "                   number of nodes = " << num_nodes << '\n'
    << "                number of elements = " << num_elmts << '\n';

  if (dimension >= 1)
  {
    if (!coord_names.empty())
    {
      s << "             first coordinate name = " << coord_names[0] << '\n';
    }
    else
    {
      s << "             first coordinate name = \n";
    }
  }
  if (dimension >= 2)
  {
    if (coord_names.size() >= 2)
    {
      s << "            second coordinate name = " << coord_names[1] << '\n';
    }
    else
    {
      s << "            second coordinate name = \n";
    }
  }
  if (dimension >= 3)
  {
    if (coord_names.size() >= 3)
    {
      s << "             third coordinate name = " << coord_names[2] << '\n';
    }
    else
    {
      s << "             third coordinate name = \n";
    }
  }

  s << separator << '\n';
  s << "          number of element blocks = " << num_elmt_blocks << '\n'
    << "              number of nodes sets = " << num_node_sets << '\n'
    << "               number of side sets = " << num_side_sets << '\n';

  if (num_elmt_blocks)
  {
    s << separator << '\n';

    s << "                   ELEMENT BLOCKS\n";
    s << "\tIndex \tId     num elmts    nodes/elmt num attr  type\n";
    for (size_t b = 0; b < num_elmt_blocks; ++b)
    {
      s << "\t" << b << "   \t" << eblocks[b].Id() << "  \t" << eblocks[b].Size() << "  \t\t"
        << eblocks[b].num_nodes_per_elmt << "  \t  " << eblocks[b].attr_count() << "  \t "
        << eblocks[b].elmt_type << '\n';
    }
  }

  if (num_node_sets)
  {
    s << separator << '\n';

    s << "              NODE SETS \n"
      << "\tIndex \tId     length \tdistribution factors length\n";
    for (size_t nset = 0; nset < num_node_sets; ++nset)
    {
      s << "\t" << nset << "  \t" << nsets[nset].Id() << "  \t" << nsets[nset].Size() << "  \t"
        << nsets[nset].num_dist_factors << '\n';
    }
  }

  if (num_side_sets)
  {
    s << separator << '\n';

    s << "              SIDE SETS \n"
      << "\tIndex \tId     length \tdistribution factors length\n";
    for (size_t sset = 0; sset < num_side_sets; ++sset)
    {
      s << "\t" << sset << "  \t" << ssets[sset].Id() << "  \t" << ssets[sset].Size() << "  \t"
        << ssets[sset].num_dist_factors << '\n';
    }
  }

  if (io_word_size || db_version > 0.0 || api_version > 0.0)
  {
    s << separator << '\n';
  }
  if (io_word_size)
  {
    s << "                  file's data size = " << io_word_size << " bytes\n";
  }
  if (db_version > 0.0)
  {
    s << "           Exodus database version = " << db_version << '\n';
  }
  if (api_version > 0.0)
  {
    s << "            Exodus library version = " << api_version << '\n';
  }
  s << separator << '\n';

  if (nodes)
  {
    s << "\tNodal Coordinates:\n";
    for (size_t n = 0; n < num_nodes; ++n)
    {
      s << "\t" << (n + 1) << "\t" << nodes[n];
      if (dimension > 1)
      {
        s << "\t" << nodes[num_nodes + n];
      }
      if (dimension > 2)
      {
        s << "\t" << nodes[num_nodes * 2 + n];
      }
      s << '\n';
    }
    s << separator << '\n';
  }

  if (!elmt_atts.empty())
  {
    s << "         number element attributes = " << elmt_atts.size() << '\n';
  }

  if (num_times)
  { // Use this to indicate whether results data exists.
    s << "\t\tRESULTS INFO\n" << separator << '\n';

    s << "           number global variables = " << global_vars.size() << '\n'
      << "            number nodal variables = " << nodal_vars.size() << '\n'
      << "          number element variables = " << elmt_vars.size() << '\n';

    unsigned max = global_vars.size() > nodal_vars.size() ? global_vars.size() : nodal_vars.size();
    max = elmt_vars.size() > max ? elmt_vars.size() : max;

    if (max != 0u)
    {
      s << "\t  GLOBAL    \t  NODAL    \t  ELEMENT\n";
    }
    for (unsigned i = 0; i < max; ++i)
    {
      if (i < global_vars.size())
      {
        s << "\t    " << global_vars[i];
      }
      else
      {
        s << "\t          ";
      }
      if (i < nodal_vars.size())
      {
        s << "\t    " << nodal_vars[i];
      }
      else
      {
        s << "\t          ";
      }
      if (i < elmt_vars.size())
      {
        s << "\t    " << elmt_vars[i] << '\n';
      }
      else
      {
        s << '\n';
      }
    }

    s << separator << '\n';

    s << "                   number of times = " << num_times << '\n';
    for (int t = 0; t < num_times; ++t)
    {
      s << "\t\t(" << (t + 1) << ") " << times[t] << '\n';
    }
  }
}

template <typename INT>
void
ExoII_Read<INT>::Display_Maps(std::ostream & s) const
{
  SMART_ASSERT(Check_State());

  s << "ExoII_Read::Display_Maps()  file name = " << file_name << '\n';
  if (node_map == nullptr)
  {
    s << "                       (node map is not loaded)\n";
  }
  if (elmt_map == nullptr)
  {
    s << "                       (elmt map is not loaded)\n";
  }
  if (node_map != nullptr || elmt_map != nullptr)
  {
    s << "\tNode Map\tElmt Map\n";

    size_t max = num_nodes > num_elmts ? num_nodes : num_elmts;
    for (size_t i = 0; i < max; ++i)
    {
      s << "   " << (i + 1) << ")\t";
      if (i < num_nodes && node_map != nullptr)
      {
        s << node_map[i];
      }
      s << "\t";
      if (i < num_elmts && elmt_map != nullptr)
      {
        s << elmt_map[i];
      }
      s << '\n';
    }
  }
}

template <typename INT>
int
ExoII_Read<INT>::Check_State() const
{
  SMART_ASSERT(file_id >= -1);
  SMART_ASSERT(db_version >= 0.0);
  SMART_ASSERT(api_version >= 0.0);
  SMART_ASSERT(io_word_size == 0 || io_word_size == 4 || io_word_size == 8);

  SMART_ASSERT(!(file_id >= 0 && io_word_size == 0));
  SMART_ASSERT(!(file_id >= 0 && file_name == ""));

  SMART_ASSERT(!(num_elmt_blocks > 0 && !eblocks));
  SMART_ASSERT(!(num_node_sets > 0 && !nsets));
  SMART_ASSERT(!(num_side_sets > 0 && !ssets));

  SMART_ASSERT(!(num_nodes == 0 && nodes));

  SMART_ASSERT(num_times >= 0);
  SMART_ASSERT(!(num_times > 0 && !times));

  SMART_ASSERT(cur_time >= 0 && cur_time <= num_times);
  SMART_ASSERT(!(!nodal_vars.empty() && !results));
  SMART_ASSERT(!(nodal_vars.empty() && results));

  return 1;
}

template <typename INT>
std::string
ExoII_Read<INT>::File_Name(const char * fname)
{
  SMART_ASSERT(Check_State());

  if (Open())
  {
    return "exodiff: ERROR: File is already open!";
  }
  if ((fname == nullptr) || std::strlen(fname) == 0)
  {
    return "exodiff: ERROR: File name is empty!";
  }
  file_name = fname;

  return "";
}

template <typename INT>
std::string
ExoII_Read<INT>::Open_File(const char * fname)
{
  SMART_ASSERT(Check_State());

  if (Open())
  {
    return "exodiff: ERROR: File already open!";
  }
  if ((fname != nullptr) && std::strlen(fname) > 0)
  {
    file_name = fname;
  }
  else if (file_name == "")
  {
    return "exodiff: ERROR: No file name to open!";
  }
  int ws = 0, comp_ws = 8;
  float dum = 0.0;
  int mode = EX_READ;
  if (sizeof(INT) == 8)
  {
    mode |= EX_ALL_INT64_API;
  }
  int err = ex_open(file_name.c_str(), mode, &comp_ws, &ws, &dum);
  if (err < 0)
  {
    std::ostringstream oss;
    oss << "exodiff: ERROR: Couldn't open file \"" << file_name << "\".";

    // ExodusII library could not open file.  See if a file (exodusII
    // or not) exists with the specified name.
    FILE * fid = fopen(file_name.c_str(), "r");
    if (fid != nullptr)
    {
      oss << " File exists, but is not an exodusII file.";
      fclose(fid);
    }
    else
    {
      oss << " File does not exist.";
    }
    return oss.str();
  }

  file_id = err;
  io_word_size = ws;

  Get_Init_Data();

  return "";
}

template <typename INT>
void
ExoII_Read<INT>::Get_Init_Data()
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(file_id >= 0);

  // Determine max size of entity and variable names on the database
  int name_length = ex_inquire_int(file_id, EX_INQ_DB_MAX_USED_NAME_LENGTH);
  ex_set_max_name_length(file_id, name_length);

  ex_init_params info{};
  info.title[0] = '\0';

  int err = ex_get_init_ext(file_id, &info);
  if (err < 0)
  {
    ERROR("Failed to get init data!"
          << " Error number = " << err << ".  Aborting...\n");
    exit(1);
  }

  dimension = info.num_dim;
  num_nodes = info.num_nodes;
  num_elmts = info.num_elem;
  num_elmt_blocks = info.num_elem_blk;
  num_node_sets = info.num_node_sets;
  num_side_sets = info.num_side_sets;
  title = info.title;

  if (err > 0 && !interface.quiet_flag)
  {
    std::cerr << "exodiff: WARNING: was issued, number = " << err << '\n';
  }
  if (dimension < 1 || dimension > 3)
  {
    ERROR("Init data appears corrupt:\n"
          << "         dimension = " << dimension << '\n'
          << "         num_nodes = " << num_nodes << '\n'
          << "         num_elmts = " << num_elmts << '\n'
          << "         num_elmt_blocks = " << num_elmt_blocks << '\n'
          << "         num_node_sets = " << num_node_sets << '\n'
          << "         num_side_sets = " << num_side_sets << '\n'
          << " ... Aborting...\n");
    exit(1);
  }

  int num_qa = ex_inquire_int(file_id, EX_INQ_QA);
  int num_info = ex_inquire_int(file_id, EX_INQ_INFO);

  if (num_qa < 0 || num_info < 0)
  {
    ERROR("inquire data appears corrupt:\n"
          << "         num_qa = " << num_qa << '\n'
          << "         num_info = " << num_info << '\n'
          << " ... Aborting...\n");
    exit(1);
  }

  //                   Coordinate Names...

  char ** coords = get_name_array(3, name_length);
  err = ex_get_coord_names(file_id, coords);
  if (err < 0)
  {
    ERROR("Failed to get coordinate"
          << " names!  Aborting...\n");
    exit(1);
  }

  coord_names.clear();
  for (size_t i = 0; i < dimension; ++i)
  {
    coord_names.push_back(coords[i]);
  }
  free_name_array(coords, 3);

  //                 Element Block Data...

  if (eblocks)
  {
    delete[] eblocks;
  }
  eblocks = nullptr;
  if (num_elmt_blocks > 0)
  {
    eblocks = new Exo_Block<INT>[num_elmt_blocks];
    SMART_ASSERT(eblocks != nullptr);
    std::vector<INT> ids(num_elmt_blocks);

    err = ex_get_ids(file_id, EX_ELEM_BLOCK, TOPTR(ids));

    if (err < 0)
    {
      ERROR("Failed to get element"
            << " block ids!  Aborting...\n");
      exit(1);
    }

    size_t e_count = 0;
    for (size_t b = 0; b < num_elmt_blocks; ++b)
    {
      if (ids[b] <= EX_INVALID_ID)
      {
        std::cerr << "EXODIFF  WARNING:  Element block Id "
                  << "for block index " << b << " is " << ids[b]
                  << " which is negative. This was returned by call to ex_get_elem_blk_ids()."
                  << '\n';
      }

      eblocks[b].initialize(file_id, ids[b]);
      e_count += eblocks[b].Size();
    }

    if (e_count != num_elmts && !interface.quiet_flag)
    {
      std::cerr << "exodiff: WARNING: Total number of elements " << num_elmts
                << " does not equal the sum of the number of elements "
                << "in each block " << e_count << '\n';
    }

    // Gather the attribute names (even though not all attributes are on all blocks)
    std::set<std::string> names;
    for (size_t b = 0; b < num_elmt_blocks; ++b)
    {
      for (int a = 0; a < eblocks[b].attr_count(); a++)
      {
        names.insert(eblocks[b].Get_Attribute_Name(a));
      }
    }
    elmt_atts.resize(names.size());
    std::copy(names.begin(), names.end(), elmt_atts.begin());
  }

  //                     Node & Side sets...

  if (nsets)
  {
    delete[] nsets;
  }
  nsets = nullptr;
  if (num_node_sets > 0)
  {
    nsets = new Node_Set<INT>[num_node_sets];
    SMART_ASSERT(nsets != nullptr);
    std::vector<INT> ids(num_node_sets);

    err = ex_get_ids(file_id, EX_NODE_SET, TOPTR(ids));

    if (err < 0)
    {
      ERROR("Failed to get nodeset ids!  Aborting...\n");
      exit(1);
    }

    for (size_t nset = 0; nset < num_node_sets; ++nset)
    {
      if (ids[nset] <= EX_INVALID_ID)
      {
        std::cerr << "EXODIFF  WARNING: Nodeset Id "
                  << "for nodeset index " << nset << " is " << ids[nset]
                  << " which is negative.  This was returned by call to ex_get_ids().\n";
      }

      nsets[nset].initialize(file_id, ids[nset]);
    }
  }

  if (ssets)
  {
    delete[] ssets;
  }
  ssets = nullptr;
  if (num_side_sets)
  {
    ssets = new Side_Set<INT>[num_side_sets];
    SMART_ASSERT(ssets != nullptr);
    std::vector<INT> ids(num_side_sets);

    err = ex_get_ids(file_id, EX_SIDE_SET, TOPTR(ids));

    if (err < 0)
    {
      ERROR("Failed to get sideset ids!  Aborting...\n");
      exit(1);
    }

    for (size_t sset = 0; sset < num_side_sets; ++sset)
    {
      if (ids[sset] <= EX_INVALID_ID)
      {
        std::cerr << "EXODIFF  WARNING:  Sideset Id "
                  << "for sideset index " << sset << " is " << ids[sset]
                  << " which is negative. This was returned by call to ex_get_ids().\n";
      }
      ssets[sset].initialize(file_id, ids[sset]);
    }
  }

  //  **************  RESULTS info  ***************  //

  int num_global_vars, num_nodal_vars, num_elmt_vars, num_ns_vars, num_ss_vars;

  err = ex_get_variable_param(file_id, EX_GLOBAL, &num_global_vars);
  if (err < 0)
  {
    ERROR("Failed to get number of global variables!  Aborting...\n");
    exit(1);
  }

  err = ex_get_variable_param(file_id, EX_NODAL, &num_nodal_vars);
  if (err < 0)
  {
    ERROR("Failed to get number of nodal variables!  Aborting...\n");
    exit(1);
  }

  err = ex_get_variable_param(file_id, EX_ELEM_BLOCK, &num_elmt_vars);
  if (err < 0)
  {
    ERROR("Failed to get number of element variables!  Aborting...\n");
    exit(1);
  }

  err = ex_get_variable_param(file_id, EX_NODE_SET, &num_ns_vars);
  if (err < 0)
  {
    ERROR("Failed to get number of nodeset variables!  Aborting...\n");
    exit(1);
  }

  err = ex_get_variable_param(file_id, EX_SIDE_SET, &num_ss_vars);
  if (err < 0)
  {
    ERROR("Failed to get number of sideset variables!  Aborting...\n");
    exit(1);
  }

  if (num_global_vars < 0 || num_nodal_vars < 0 || num_elmt_vars < 0 || num_ns_vars < 0 ||
      num_ss_vars < 0)
  {
    ERROR("Data appears corrupt for"
          << " number of variables !\n"
          << "\tnum global vars  = " << num_global_vars << '\n'
          << "\tnum nodal vars   = " << num_nodal_vars << '\n'
          << "\tnum element vars = " << num_elmt_vars << '\n'
          << " ... Aborting...\n");
    exit(1);
  }

  read_vars(file_id, EX_GLOBAL, "Global", num_global_vars, global_vars);
  read_vars(file_id, EX_NODAL, "Nodal", num_nodal_vars, nodal_vars);
  read_vars(file_id, EX_ELEM_BLOCK, "Element", num_elmt_vars, elmt_vars);
  read_vars(file_id, EX_NODE_SET, "Nodeset", num_ns_vars, ns_vars);
  read_vars(file_id, EX_SIDE_SET, "Sideset", num_ss_vars, ss_vars);

  // Times:
  num_times = ex_inquire_int(file_id, EX_INQ_TIME);
  if (num_times < 0)
  {
    ERROR("Number of time steps came back negative (" << num_times << ")!  Aborting...\n");
    exit(1);
  }

  if ((num_global_vars > 0 || num_nodal_vars > 0 || num_elmt_vars > 0 || num_ns_vars > 0 ||
       num_ss_vars > 0) &&
      num_times == 0)
  {
    ERROR("Consistency error -- The database contains transient variables, but no "
          "timesteps!\n");
    exit(1);
  }

  if (num_times)
  {
    times = new double[num_times];
    SMART_ASSERT(times != nullptr);
    err = ex_get_all_times(file_id, times);
  }

  if (num_nodal_vars != 0)
  {
    if (num_times == 0)
    {
      ERROR("Consistency error--The database contains "
            << num_nodal_vars << " nodal variables, but there are no time steps defined.\n");
    }
    if (num_times)
    {
      results = new double *[num_nodal_vars];
      for (int i = 0; i < num_nodal_vars; ++i)
      {
        results[i] = nullptr;
      }
    }
  }

} // End of EXODIFF

namespace
{
void
read_vars(
    int file_id, EXOTYPE flag, const char * type, int num_vars, std::vector<std::string> & varlist)
{
  if (num_vars != 0)
  {
    int name_size = ex_inquire_int(file_id, EX_INQ_MAX_READ_NAME_LENGTH);
    char ** varnames = get_name_array(num_vars, name_size);
    int err = ex_get_variable_names(file_id, flag, num_vars, varnames);

    if (err < 0)
    {
      ERROR("Failed to get " << type << " variable names!  Aborting...\n");
      exit(1);
    }
    else if (err > 0 && !interface.quiet_flag)
    {
      std::cerr << "exodiff: WARNING: Exodus issued warning "
                << "\"" << err << "\" on call to ex_get_var_names()!\n";
    }
    for (int vg = 0; vg < num_vars; ++vg)
    {
      SMART_ASSERT(varnames[vg] != nullptr);
      if (std::strlen(varnames[vg]) == 0 || static_cast<int>(std::strlen(varnames[vg])) > name_size)
      {
        std::ostringstream out;
        out << "exodiff: ERROR: " << type << " variable names appear corrupt\n"
            << "                A length is 0 or greater than "
            << "name_size(" << name_size << ")\n"
            << "                Here are the names that I received from"
            << " a call to ex_get_var_names(...):\n";
        for (int k = 1; k <= num_vars; ++k)
        {
          out << "\t\t" << k << ") \"" << varnames[k - 1] << "\"\n";
        }
        out << "                 Aborting...\n";
        DIFF_OUT(out);
        exit(1);
      }

      std::string n(varnames[vg]);
      chop_whitespace(n);
      varlist.push_back(n);
    }
    free_name_array(varnames, num_vars);
  }
}
} // namespace
template class ExoII_Read<int>;
template class ExoII_Read<int64_t>;
