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

#ifndef EXOII_READ_H
#define EXOII_READ_H

#include "exodusII.h"
#include "netcdf.h"  
#include "exo_entity.h"

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
class Exo_Block;
class Node_Set;
class Side_Set;


class ExoII_Read {
public:
  
  ExoII_Read();
  explicit ExoII_Read(const char* file_name);
  virtual ~ExoII_Read();
  const ExoII_Read& operator=(const ExoII_Read&);  // Not written.
  
  // File operations:
  
  std::string File_Name(const char*);
  virtual std::string Open_File(const char* = 0);  // Default opens current file name.
  std::string Close_File();
  std::string File_Name()    const { return file_name; }
  int     Open()         const { return (file_id >= 0); }
  int     IO_Word_Size() const { return io_word_size;   }
  
  // Global data:
  
  const std::string& Title() const { return title;        }
  int     Dimension()       const { return dimension;     }
  int     Num_Nodes()       const { return num_nodes;     }
  int     Num_Elmts()       const { return num_elmts;     }
  int     Num_Node_Sets()   const { return num_node_sets; }
  int     Num_Side_Sets()   const { return num_side_sets; }
  float Data_Base_Version() const { return db_version;    }
  float Library_Version()   const { return api_version;   }
  const std::vector<std::string>& Coordinate_Names() const { return coord_names; }
        std::string               Coordinate_Name (unsigned i) const;
  
  // Times:
  
  int Num_Times() const { return num_times; }
  double Time(int time_num) const;
  
  // Variables:
  
  int Num_Global_Vars() const { return global_vars.size(); }
  int Num_Nodal_Vars()  const { return nodal_vars.size();  }
  int Num_Elmt_Vars()   const { return elmt_vars.size();   }
  int Num_Elmt_Atts()   const { return elmt_atts.size();   }
  int Num_NS_Vars()     const { return ns_vars.size();   }
  int Num_SS_Vars()     const { return ss_vars.size();   }
  const std::vector<std::string>& Global_Var_Names() const { return global_vars; }
  const std::vector<std::string>& Nodal_Var_Names()  const { return nodal_vars;  }
  const std::vector<std::string>& Elmt_Var_Names()   const { return elmt_vars;   }
  const std::vector<std::string>& Elmt_Att_Names()   const { return elmt_atts;   }
  const std::vector<std::string>& NS_Var_Names()     const { return ns_vars;     }
  const std::vector<std::string>& SS_Var_Names()     const { return ss_vars;     }
  const std::string&      Global_Var_Name(int index) const;
  const std::string&      Nodal_Var_Name (int index) const;
  const std::string&      Elmt_Var_Name  (int index) const;
  const std::string&      Elmt_Att_Name  (int index) const;
  const std::string&      NS_Var_Name    (int index) const;
  const std::string&      SS_Var_Name    (int index) const;
  
  
  // Element blocks:
  int Num_Elmt_Blocks() const { return num_elmt_blocks; }
  
  std::string Load_Elmt_Block_Description(int block_index) const;
  std::string Load_Elmt_Block_Descriptions() const;  // Loads all blocks.
  std::string Load_Elmt_Block_Results(int block_index,
                                      int elmt_var_index,
                                      int time_step_num) const;
  std::string Free_Elmt_Block(int block_index) const; // Frees all dynamic memory.
  std::string Free_Elmt_Blocks() const;               // Frees all blocks.
  std::string Free_Elmt_Block_Results(int block_index) const;
  
  // Moves array of connectivities from the block to the conn array.
  std::string Give_Connectivity(int block_index, int& num_e, int& npe, int*& conn);
  
  // Number maps:
  std::string Load_Node_Map();
  std::string Free_Node_Map();
  const int*  Get_Node_Map() { return node_map; }
  std::string Load_Elmt_Map();
  std::string Free_Elmt_Map();
  const int*  Get_Elmt_Map() { return elmt_map; }
  std::string Load_Elmt_Order();
  std::string Free_Elmt_Order();
  const int*  Get_Elmt_Order() { return elmt_order; }
  void Free_All_Maps();
  inline int Node_Map  (int node_num) const;  // numbers are global, 1-offset
  inline int Elmt_Map  (int elmt_num) const;  // numbers are global, 1-offset
  inline int Elmt_Order(int elmt_num) const;  // numbers are global, 1-offset
  
  
  // Nodal data:
  
  std::string Load_Nodal_Coordinates();
  const double* X_Coords() const { return nodes; }
  const double* Y_Coords() const { if (dimension < 2) return 0;
                                   return nodes == 0 ? 0: nodes + num_nodes; }
  const double* Z_Coords() const { if (dimension < 3) return 0;
                                   return nodes == 0 ? 0: nodes + 2*num_nodes; }
  void Free_Nodal_Coordinates();
  
  // (First time step = 1.)
  std::string Load_Nodal_Results(int time_step_num, int var_index);
  const double* Get_Nodal_Results(int var_index) const;
  void Free_Nodal_Results();
  
  // Global data:  (NOTE:  Global and Nodal data are always stored at the same
  //                       time step.  Therefore, if current time step number
  //                       is changed, the results will all be deleted.)
  
  std::string Load_Global_Results(int time_step_num);
  const double* Get_Global_Results() const { return global_vals; }
  
  // Node/Side sets:
  

  Exo_Entity* Get_Entity_by_Index(EXOTYPE type, int index) const;
  Exo_Entity* Get_Entity_by_Id   (EXOTYPE type, int id)    const;
  
  int        Block_Index            (int block_id) const;  // Returns associated block index.
  int        Block_Id               (int block_index) const;  // Returns associated block id.
  Exo_Block* Get_Elmt_Block_by_Id   (int block_id)    const;
  Exo_Block* Get_Elmt_Block_by_Index(int block_index) const;
  

  int       Side_Set_Index       (int set_id)         const;  // Returns associated sideset index.
  int       Side_Set_Id          (int set_index)      const;
  Side_Set* Get_Side_Set_by_Id   (int set_id)         const;
  Side_Set* Get_Side_Set_by_Index(int side_set_index) const;

  int       Node_Set_Index       (int set_id)         const;  // Returns associated sideset index.
  int       Node_Set_Id          (int set_index)      const;
  Node_Set* Get_Node_Set_by_Id   (int set_id)         const;
  Node_Set* Get_Node_Set_by_Index(int side_set_index) const;
  
  std::string Load_NS_Results(int set_index, int var_index, int time_step_num) const;
  std::string Free_NS_Results(int set_index) const;
  std::string Load_SS_Results(int set_index, int var_index, int time_step_num) const;
  std::string Free_SS_Results(int set_index) const;
  
  // Misc functions:
  
  virtual void Display_Stats(std::ostream& = std::cout) const;
  virtual void Display      (std::ostream& = std::cout) const;
  virtual void Display_Maps (std::ostream& = std::cout) const;
  virtual int  Check_State() const;  // Checks state of obj (not the file).
  int  File_ID() const { return file_id; }  // This is temporary.
  std::string Global_to_Block_Local(int global_elmt_num,            // 1-offset
                                    int& block_index,               // 0-offset
                                    int& local_elmt_index) const;   // 0-offset
  
protected:
  
  std::string file_name;
  int     file_id;    // Exodus file id; also used to determine if file is open.
  
  // GENESIS info:
  
  std::string  title;
  int          num_nodes;
  std::vector<std::string> coord_names;
  int          dimension;
  int          num_elmts;
  int          num_elmt_blocks;
  int          num_node_sets;
  int          num_side_sets;
  float        db_version;
  float        api_version;
  int          io_word_size;    // Note: The "compute word size" is always 8.
  
  Exo_Block* eblocks;   // Array.
  Node_Set*  nsets;     // Array.
  Side_Set*  ssets;     // Array.
  
  double* nodes;        // Matrix;  dimension by num_nodes (row major form).
                        //          I.e., all x's then all y's, etc.
  
  int* node_map;        // Array; num_nodes long when filled.
  int* elmt_map;        // Array; num_elmts long when filled.
  int* elmt_order;      // Array; num_elmts long when filled.
  
  // RESULTS info:
  
  std::vector<std::string> global_vars;
  std::vector<std::string> nodal_vars;
  std::vector<std::string> elmt_vars;
  std::vector<std::string> elmt_atts;
  std::vector<std::string> ns_vars;
  std::vector<std::string> ss_vars;
  
  int     num_times;
  double* times;
  
  int cur_time;      // Current timestep number of the results (0 means none).
  double** results;  // Array of pointers (to arrays of results data);
                     // length is number of nodal variables.
  double* global_vals;  // Array of global variables for the current timestep.
  
  // Internal methods:
  
  void Get_Init_Data();         // Gets bunch of initial data.
  
  int Elmt_Block_Index(int eblock_id) const;  // Returns index of element block.
  int NSet_Index(int node_set_id) const;      // Returns index of node set.
  int SSet_Index(int side_set_id) const;      // Returns index of side set.
  
  int File_Exists(const char* fname);

 private:
  ExoII_Read(const ExoII_Read&);  // Not written.
  
};

inline int ExoII_Read::Node_Map(int node_num) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(node_num > 0 && node_num <= num_nodes);
  
  if (node_map) return node_map[ node_num - 1 ];
  return 0;
}

inline int ExoII_Read::Elmt_Map(int elmt_num) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(elmt_num > 0 && elmt_num <= num_elmts);
  
  if (elmt_map) return elmt_map[ elmt_num - 1 ];
  return 0;
}

inline int ExoII_Read::Elmt_Order(int elmt_num) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(elmt_num > 0 && elmt_num <= num_elmts);
  
  if (elmt_order) return elmt_order[ elmt_num - 1 ];
  return 0;
}


#endif
