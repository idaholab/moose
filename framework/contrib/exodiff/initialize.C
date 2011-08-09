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
#include <fstream>
#include <vector>
#include <set>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "smart_assert.h"
#include "exoII_read.h"
#include "exo_block.h"
#include "node_set.h"
#include "side_set.h"
#include "exodusII.h"
#include "stringx.h"
#include "Specifications.h"
#include "util.h"

using namespace std;

namespace {
  float   inquire_float(int exo_file_id,  int request);
  void read_vars(int file_id, EXOTYPE flag, const char *type,
		 int num_vars, vector<string> &varlist);

}
    

string ExoII_Read::File_Name(const char* fname)
{
  SMART_ASSERT(Check_State());
  
  if (Open()) return  "ERROR: File is already open!";
  if (!fname || std::strlen(fname) == 0) return "ERROR: File name is empty!";
  
  file_name = fname;
  
  return "";
}

string ExoII_Read::Open_File(const char* fname)
{
  SMART_ASSERT(Check_State());
  
  if (Open()) return "ERROR: File already open!";
  
  if (fname && std::strlen(fname) > 0)
    file_name = fname;
  else if (file_name == "")
    return "ERROR: No file name to open!";
  
  int ws = 0, comp_ws = 8;
  float dum = 0.0;
  int err = ex_open(file_name.c_str(), EX_READ,
                    &comp_ws, &ws, &dum);
  if (err < 0) {
    ostringstream oss;
    oss << "ERROR: Couldn't open file \"" << file_name << "\".";
    
    // ExodusII library could not open file.  See if a file (exodusII
    // or not) exists with the specified name.
    FILE *fid = fopen(file_name.c_str(),"r");
    if (fid != NULL) {
      oss << " File exists, but is not an exodusII file.";
    } else {
      oss << " File does not exist.";
    }
    return oss.str();
  }
  
  file_id = err;
  io_word_size = ws;

  Get_Init_Data();
  
  return "";
}

void ExoII_Read::Get_Init_Data()
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(file_id >= 0);
  
  // Determine max size of entity and variable names on the database
  int name_length = ex_inquire_int(file_id, EX_INQ_DB_MAX_USED_NAME_LENGTH);
  ex_set_max_name_length(file_id, name_length);
  
  char title_buff[MAX_LINE_LENGTH+1];
  
  int err = ex_get_init(file_id, title_buff, &dimension, &num_nodes,
                        &num_elmts, &num_elmt_blocks, &num_node_sets,
                        &num_side_sets);
  if (err < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get init data!"
	      << " Error number = " << err << ".  Aborting..." << std::endl;
    exit(1);
  }
  title = title_buff;
  if (err > 0 && !specs.quiet_flag)
    std::cout << "ExoII_Read::Get_Init_Data(): WARNING: was issued, number = "
	      << err << std::endl;
  if (dimension < 1 || dimension > 3 ||
      num_nodes < 0 || num_elmts < 0 || num_elmt_blocks < 0 ||
      num_node_sets < 0 || num_side_sets < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Init data appears corrupt:"
	      << std::endl
	      << "         dimension = "       << dimension       << std::endl
	      << "         num_nodes = "       << num_nodes       << std::endl
	      << "         num_elmts = "       << num_elmts       << std::endl
	      << "         num_elmt_blocks = " << num_elmt_blocks << std::endl
	      << "         num_node_sets = "   << num_node_sets   << std::endl
	      << "         num_side_sets = "   << num_side_sets   << std::endl
	      << " ... Aborting..." << std::endl;
    exit(1);
  }
  
  db_version   = inquire_float(file_id, EX_INQ_DB_VERS);
  api_version  = inquire_float(file_id, EX_INQ_API_VERS);
  int num_qa   = ex_inquire_int(file_id,   EX_INQ_QA);
  int num_info = ex_inquire_int(file_id,   EX_INQ_INFO);
  
  if (db_version < 0.0 || api_version < 0.0 || num_qa < 0 || num_info < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: inquire data appears corrupt:"
	      << std::endl
	      << "         db_version = "  << db_version  << std::endl
	      << "         api_version = " << api_version << std::endl
	      << "         num_qa = "      << num_qa      << std::endl
	      << "         num_info = "    << num_info    << std::endl
	      << " ... Aborting..." << std::endl;
    exit(1);
  }
  
  //                   Coordinate Names...
  
  char** coords = get_name_array(3, name_length);
  err = ex_get_coord_names(file_id, coords);
  if (err < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get coordinate"
	      << " names!  Aborting..." << std::endl;
    exit(1);
  }
  
  coord_names.clear();
  for (int i = 0; i < dimension; ++i) {
    coord_names.push_back(coords[i]);
  }
  free_name_array(coords, 3);
  
  //                 Element Block Data...
  
  if (eblocks) delete [] eblocks;  eblocks = 0;
  if (num_elmt_blocks > 0) {
    eblocks = new Exo_Block[num_elmt_blocks];    SMART_ASSERT(eblocks != 0);
    std::vector<int> ids(num_elmt_blocks);
    
    err = ex_get_ids(file_id, EX_ELEM_BLOCK, &ids[0]);
    
    if (err < 0) {
      std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get element"
		<< " block ids!  Aborting..." << std::endl;
      exit(1);
    }
    
    int e_count = 0;
    for (int b = 0; b < num_elmt_blocks; ++b) {
      if (ids[b] <= EX_INVALID_ID) {
	std::cout << "ExoII_Read::Get_Init_Data()  WARNING:  Element block Id "
		  << "for block index " << b << " is " << ids[b]
		  << " which is negative. This was returned by call to ex_get_elem_blk_ids()."
		  << std::endl;
      }
      
      eblocks[b].initialize(file_id, ids[b]);
      e_count += eblocks[b].Size();
    }
    
    if (e_count != num_elmts && !specs.quiet_flag) {
      std::cout << "ExoII_Read::Get_Init_Data(): WARNING: Total number of elements "
		<< num_elmts << " does not equal the sum of the number of elements "
		<< "in each block " << e_count << std::endl;
    }

    // Gather the attribute names (even though not all attributes are on all blocks)
    std::set<std::string> names;
    for (int b = 0; b < num_elmt_blocks; ++b) {
      for (int a = 0; a < eblocks[b].attr_count(); a++) {
	names.insert(eblocks[b].Get_Attribute_Name(a));
      }
    }
    elmt_atts.resize(names.size());
    std::copy(names.begin(), names.end(), elmt_atts.begin());
  }
  
  //                     Node & Side sets...
  
  if (nsets) delete [] nsets;  nsets = 0;
  if (num_node_sets > 0) {
    nsets = new Node_Set[num_node_sets];         SMART_ASSERT(nsets != 0);
    std::vector<int> ids(num_node_sets);
    
    err = ex_get_ids(file_id, EX_NODE_SET, &ids[0]);

    if (err < 0) {
      std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get "
		<< "nodeset ids!  Aborting..." << std::endl;
      exit(1);
    }

    for (int nset = 0; nset < num_node_sets; ++nset) {
      if (ids[nset] <= EX_INVALID_ID) {
	std::cout << "ExoII_Read::Get_Init_Data()  WARNING: Nodeset Id "
		  << "for nodeset index " << nset << " is " << ids[nset]
		  << " which is negative.  This was returned by call to ex_get_ids()."
		  << std::endl;
      }
	
      nsets[nset].initialize(file_id, ids[nset]);
    }
  }
  
  if (ssets) delete [] ssets;  ssets = 0;
  if (num_side_sets) {
    ssets = new Side_Set[num_side_sets];         SMART_ASSERT(ssets != 0);
    std::vector<int> ids(num_side_sets);
    
    err = ex_get_ids(file_id, EX_SIDE_SET, &ids[0]);

    if (err < 0) {
      std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get "
		<< "sideset ids!  Aborting..." << std::endl;
      exit(1);
    }

    for (int sset = 0; sset < num_side_sets; ++sset) {
      if (ids[sset] <= EX_INVALID_ID) {
	std::cout << "ExoII_Read::Get_Init_Data()  WARNING:  Sideset Id "
		  << "for sideset index " << sset << " is " << ids[sset]
		  << " which is negative. This was returned by call to ex_get_ids()."
		  << std::endl;
      }
      ssets[sset].initialize(file_id, ids[sset]);
    }
  }
  
  
  //  **************  RESULTS info  ***************  //
  
  int num_global_vars, num_nodal_vars, num_elmt_vars, num_ns_vars, num_ss_vars;
  
  err = ex_get_variable_param(file_id, EX_GLOBAL, &num_global_vars);
  if (err < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get number of"
	      << " global variables!  Aborting..." << std::endl;
    exit(1);
  }
  
  err = ex_get_variable_param(file_id, EX_NODAL, &num_nodal_vars);
  if (err < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get number of"
	      << " nodal variables!  Aborting..." << std::endl;
    exit(1);
  }
  
  err = ex_get_variable_param(file_id, EX_ELEM_BLOCK, &num_elmt_vars);
  if (err < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get number of"
	      << " element variables!  Aborting..." << std::endl;
    exit(1);
  }
  
  err = ex_get_variable_param(file_id, EX_NODE_SET, &num_ns_vars);
  if (err < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get number of"
	      << " nodeset variables!  Aborting..." << std::endl;
    exit(1);
  }
  
  err = ex_get_variable_param(file_id, EX_SIDE_SET, &num_ss_vars);
  if (err < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get number of"
	      << " sideset variables!  Aborting..." << std::endl;
    exit(1);
  }
  
  if (num_global_vars < 0 || num_nodal_vars < 0 || num_elmt_vars < 0 ||
      num_ns_vars < 0 || num_ss_vars < 0) {
    std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Data appears corrupt for"
	      << " number of variables !" << std::endl
	      << "\tnum global vars  = " << num_global_vars  << std::endl
	      << "\tnum nodal vars   = " << num_nodal_vars   << std::endl
	      << "\tnum element vars = " << num_elmt_vars    << std::endl
	      << " ... Aborting..." << std::endl;
    exit(1);
  }
  
  read_vars(file_id, EX_GLOBAL,     "Global",  num_global_vars, global_vars);
  read_vars(file_id, EX_NODAL,      "Nodal",   num_nodal_vars,  nodal_vars);
  read_vars(file_id, EX_ELEM_BLOCK, "Element", num_elmt_vars,   elmt_vars);
  read_vars(file_id, EX_NODE_SET,   "Nodeset", num_ns_vars,     ns_vars);
  read_vars(file_id, EX_SIDE_SET,   "Sideset", num_ss_vars,     ss_vars);
  
  // Times:
  num_times = ex_inquire_int(file_id, EX_INQ_TIME);
  if (num_times < 0) {
    std::cout << "ExoII_Read::Get_Init_Data()  ERROR: Number of time steps came"
	      << " back negative (" << num_times << ")!  Aborting..." << std::endl;
    exit(1);
  }

  if (num_times) {
    times = new double[num_times];  SMART_ASSERT(times != 0);
    err = ex_get_all_times(file_id, times);
  }
  
  if (num_nodal_vars && num_times) {
    results = new double*[ num_nodal_vars ];
    for (int i = 0; i < num_nodal_vars; ++i)
      results[i] = 0;
  }
  
}  // End of ExoII_Read::Get_Init_Data()


// Simple check that a file can be opened.
int ExoII_Read::File_Exists(const char* fname)
{
  if (!fname) return 0;
  if (std::strlen(fname) == 0) return 0;
  std::ifstream file_check(fname, std::ios::in);
  if (!file_check) return 0;
  file_check.close();
  return 1;
}

namespace {
  float inquire_float(int exo_file_id, int request)
  {
    SMART_ASSERT(exo_file_id >= 0);
    int   get_int = 0;
    float get_flt = 0.0;
    char  get_str[MAX_LINE_LENGTH+1];

    int err = ex_inquire(exo_file_id, request, &get_int, &get_flt, get_str);

    if (err < 0) {
      std::cout << "ExoII_Read::inquire_float(): ERROR " << err
		<< ": ex_inquire failed!  Aborting..." << std::endl;
      exit(1);
    }

    if (err > 0)
      std::cout << "ExoII_Read::inquire_float(): WARNING: " << err
		<< " issued by ex_inquire call!" << std::endl;

    return get_flt;
  }

  void read_vars(int file_id, EXOTYPE flag, const char *type,
		 int num_vars, vector<string> &varlist)
  {
    if (num_vars) {
      int name_size = ex_inquire_int(file_id, EX_INQ_MAX_READ_NAME_LENGTH);
      char **varnames = get_name_array(num_vars, name_size);
      int err = ex_get_variable_names(file_id, flag, num_vars, varnames);

      if (err < 0) {
	std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get " << type 
		  << " variable names!  Aborting..." << std::endl;
	exit(1);
      }
      else if (err > 0 && !specs.quiet_flag)
	std::cout << "ExoII_Read::Get_Init_Data(): WARNING: Exodus issued warning "
		  << "\"" << err << "\" on call to ex_get_var_names()!" << std::endl;
      for (int vg = 0; vg < num_vars; ++vg) {
	SMART_ASSERT(varnames[vg] != 0);
	if (std::strlen(varnames[vg]) == 0 ||
	    (int)std::strlen(varnames[vg]) > name_size) {
	  std::cout << "exodiff: ERROR: " << type
		    << " variable names appear corrupt\n"
		    << "                A length is 0 or greater than "
		    << "name_size(" << name_size << ")\n"
		    << "                Here are the names that I received from"
		    << " a call to ex_get_var_names(...):\n";
	  for (int k = 1; k <= num_vars; ++k)
	    std::cout << "\t\t" << k << ") \"" << varnames[k-1] << "\"\n";
	  std::cout << "                 Aborting..." << std::endl;
	  exit(1);
	}
        
        string n(varnames[vg]);
        chop_whitespace(n);
	varlist.push_back(n);
      }
      free_name_array(varnames, num_vars);
    }
  }
}
