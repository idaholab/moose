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

#ifndef EXOII_READ_H
#define EXOII_READ_H

#include "exo_entity.h"
#include "libmesh/exodusII.h"
#include "netcdf.h"

#include <iostream>
#include <string>
#include <vector>

#include "smart_assert.h"

//
//  Notes: 1) Most functions will return a string as a result.  An empty string
//            indicates success, while a non-empty string indicates an error or
//            a warning, either of which will be contained in the string.
//

class Exo_Entity;
template <typename INT>
class Exo_Block;

template <typename INT>
class Node_Set;
template <typename INT>
class Side_Set;

template <typename INT>
class ExoII_Read
{
public:
  ExoII_Read();
  explicit ExoII_Read(const char * fname);
  virtual ~ExoII_Read();
  const ExoII_Read & operator=(const ExoII_Read &); // Not written.

  // File operations:

  std::string File_Name(const char * /*fname*/);
  virtual std::string
  Open_File(const char * /*fname*/ = nullptr); // Default opens current file name.
  std::string Close_File();
  std::string File_Name() const { return file_name; }
  int Open() const { return (file_id >= 0); }
  int IO_Word_Size() const { return io_word_size; }

  // Global data:

  const std::string & Title() const { return title; }
  int Dimension() const { return dimension; }
  size_t Num_Nodes() const { return num_nodes; }
  size_t Num_Elmts() const { return num_elmts; }
  int Num_Node_Sets() const { return num_node_sets; }
  int Num_Side_Sets() const { return num_side_sets; }

  // Times:

  int Num_Times() const { return num_times; }
  double Time(int time_num) const;

  // Variables:

  int Num_Global_Vars() const { return global_vars.size(); }
  int Num_Nodal_Vars() const { return nodal_vars.size(); }
  int Num_Elmt_Vars() const { return elmt_vars.size(); }
  int Num_Elmt_Atts() const { return elmt_atts.size(); }
  int Num_NS_Vars() const { return ns_vars.size(); }
  int Num_SS_Vars() const { return ss_vars.size(); }
  const std::vector<std::string> & Global_Var_Names() const { return global_vars; }
  const std::vector<std::string> & Nodal_Var_Names() const { return nodal_vars; }
  const std::vector<std::string> & Elmt_Var_Names() const { return elmt_vars; }
  const std::vector<std::string> & Elmt_Att_Names() const { return elmt_atts; }
  const std::vector<std::string> & NS_Var_Names() const { return ns_vars; }
  const std::vector<std::string> & SS_Var_Names() const { return ss_vars; }
  const std::string & Global_Var_Name(int index) const;
  const std::string & Nodal_Var_Name(int index) const;
  const std::string & Elmt_Var_Name(int index) const;
  const std::string & Elmt_Att_Name(int index) const;
  const std::string & NS_Var_Name(int index) const;
  const std::string & SS_Var_Name(int index) const;

  // Element blocks:
  int Num_Elmt_Blocks() const { return num_elmt_blocks; }

  std::string Load_Elmt_Block_Description(size_t block_index) const;
  std::string Load_Elmt_Block_Descriptions() const;      // Loads all blocks.
  std::string Free_Elmt_Block(size_t block_index) const; // Frees all dynamic memory.
  std::string Free_Elmt_Blocks() const;                  // Frees all blocks.

  // Moves array of connectivities from the block to the conn array.
  std::string Give_Connectivity(size_t block_index, size_t & num_e, size_t & npe, INT *& new_conn);

  // Number maps:
  std::string Load_Node_Map();
  std::string Free_Node_Map();
  const INT * Get_Node_Map() { return node_map; }
  std::string Load_Elmt_Map();
  std::string Free_Elmt_Map();
  const INT * Get_Elmt_Map() { return elmt_map; }
  inline INT Node_Map(size_t node_num) const;   // numbers are global, 1-offset
  inline INT Elmt_Map(size_t elmt_num) const;   // numbers are global, 1-offset
  inline INT Elmt_Order(size_t elmt_num) const; // numbers are global, 1-offset

  // Nodal data:

  std::string Load_Nodal_Coordinates();
  const double * X_Coords() const { return nodes; }
  const double * Y_Coords() const
  {
    if (dimension < 2)
    {
      return nullptr;
    }
    return nodes == nullptr ? nullptr : nodes + num_nodes;
  }
  const double * Z_Coords() const
  {
    if (dimension < 3)
    {
      return nullptr;
    }
    return nodes == nullptr ? nullptr : nodes + 2 * num_nodes;
  }
  void Free_Nodal_Coordinates();

  // (First time step = 1.)
  std::string Load_Nodal_Results(int time_step_num, int var_index);
  const double * Get_Nodal_Results(int var_index) const;
  const double * Get_Nodal_Results(int t1,
                                   int t2,
                                   double proportion,
                                   int var_index) const; // Interpolated results
  void Free_Nodal_Results();
  void Free_Nodal_Results(int var_index);

  // Global data:  (NOTE:  Global and Nodal data are always stored at the same
  //                       time step.  Therefore, if current time step number
  //                       is changed, the results will all be deleted.)

  std::string Load_Global_Results(int time_step_num);
  std::string Load_Global_Results(int t1, int t2, double proportion); // Interpolated results
  const double * Get_Global_Results() const { return global_vals; }

  // Node/Side sets:

  Exo_Entity * Get_Entity_by_Index(EXOTYPE type, size_t block_index) const;
  Exo_Entity * Get_Entity_by_Id(EXOTYPE type, size_t id) const;
  Exo_Entity * Get_Entity_by_Name(EXOTYPE type, const std::string & name) const;

  size_t Block_Id(size_t block_index) const; // Returns associated block id.
  Exo_Block<INT> * Get_Elmt_Block_by_Id(size_t id) const;
  Exo_Block<INT> * Get_Elmt_Block_by_Index(size_t block_index) const;
  Exo_Block<INT> * Get_Elmt_Block_by_Name(const std::string & name) const;

  Side_Set<INT> * Get_Side_Set_by_Id(size_t set_id) const;
  Side_Set<INT> * Get_Side_Set_by_Index(size_t side_set_index) const;
  Side_Set<INT> * Get_Side_Set_by_Name(const std::string & name) const;

  Node_Set<INT> * Get_Node_Set_by_Id(size_t set_id) const;
  Node_Set<INT> * Get_Node_Set_by_Index(size_t set_index) const;
  Node_Set<INT> * Get_Node_Set_by_Name(const std::string & name) const;

  // Misc functions:

  virtual void Display(std::ostream & /*s*/ = std::cout) const;
  virtual void Display_Maps(std::ostream & /*s*/ = std::cout) const;
  virtual int Check_State() const;                          // Checks state of obj (not the file).
  int File_ID() const { return file_id; }                   // This is temporary.
  std::string Global_to_Block_Local(size_t global_elmt_num, // 1-offset
                                    int & block_index,      // 0-offset
                                    size_t & local_elmt_index) const; // 0-offset

protected:
  std::string file_name;
  int file_id; // Exodus file id; also used to determine if file is open.

  // GENESIS info:

  std::string title;
  std::vector<std::string> coord_names;
  size_t num_nodes;
  size_t dimension;
  size_t num_elmts;
  size_t num_elmt_blocks;
  size_t num_node_sets;
  size_t num_side_sets;
  float db_version;
  float api_version;
  int io_word_size; // Note: The "compute word size" is always 8.

  Exo_Block<INT> * eblocks; // Array.
  Node_Set<INT> * nsets;    // Array.
  Side_Set<INT> * ssets;    // Array.

  double * nodes; // Matrix;  dimension by num_nodes (row major form).
                  //          I.e., all x's then all y's, etc.

  INT * node_map;   // Array; num_nodes long when filled.
  INT * elmt_map;   // Array; num_elmts long when filled.
  INT * elmt_order; // Array; num_elmts long when filled.

  // RESULTS info:

  std::vector<std::string> global_vars;
  std::vector<std::string> nodal_vars;
  std::vector<std::string> elmt_vars;
  std::vector<std::string> elmt_atts;
  std::vector<std::string> ns_vars;
  std::vector<std::string> ss_vars;

  int num_times;
  double * times;

  int cur_time;          // Current timestep number of the results (0 means none).
  double ** results;     // Array of pointers (to arrays of results data);
                         // length is number of nodal variables.
  double * global_vals;  // Array of global variables for the current timestep.
  double * global_vals2; // Array of global variables used if interpolating.

  // Internal methods:

  void Get_Init_Data(); // Gets bunch of initial data.

private:
  ExoII_Read(const ExoII_Read &); // Not written.
};

template <typename INT>
inline INT
ExoII_Read<INT>::Node_Map(size_t node_num) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(node_num <= num_nodes);

  if (node_map)
  {
    return node_map[node_num - 1];
  }
  return 0;
}

template <typename INT>
inline INT
ExoII_Read<INT>::Elmt_Map(size_t elmt_num) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(elmt_num <= num_elmts);

  if (elmt_map)
  {
    return elmt_map[elmt_num - 1];
  }
  return 0;
}

template <typename INT>
inline INT
ExoII_Read<INT>::Elmt_Order(size_t elmt_num) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(elmt_num <= num_elmts);

  if (elmt_order)
  {
    return elmt_order[elmt_num - 1];
  }
  return 0;
}

#endif
