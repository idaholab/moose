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

#include "Tolerance.h"
#include "smart_assert.h"
#include "exoII_read.h"
#include "exo_block.h"
#include "node_set.h"
#include "side_set.h"
#include "libmesh/exodusII.h"
#include "stringx.h"
#include "ED_SystemInterface.h"
#include "util.h"

#include <cstdlib>
#include <cstdio>
#include <math.h>
#include <ctype.h>
#include <vector>
#include <string>

using namespace std;

namespace {
  void build_variable_names(const char *type, vector<string> &names, std::vector<Tolerance> &tols,
			    const Tolerance& default_tol, bool do_all_flag,
			    const vector<string> &var_names1, const vector<string> &var_names2,
			    bool *diff_found);

  template <typename INT>
  void build_truth_table(EXOTYPE type, const char *label, vector<string> &names, size_t num_entity,
			 ExoII_Read<INT>& file1, ExoII_Read<INT>& file2,
			 const vector<string> &var_names1, const vector<string> &var_names2,
			 std::vector<int> &truth_tab, bool quiet_flag,
			 bool *diff_found);

  void output_exodus_names(int file_id, EXOTYPE type, const vector<string> &names);
  void output_diff_names(const char *type, const vector<string> &names);
  void output_compare_names(const char* type, const vector<string> &names, const std::vector<Tolerance> &tol,
			    int num_vars1, int num_vars2);
}

namespace {
char buf[256];
}

template <typename INT>
void Build_Variable_Names(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, bool *diff_found)
{
  // Build (and compare) global variable names.
  build_variable_names("global", interface.glob_var_names, interface.glob_var,
                       interface.glob_var_default, interface.glob_var_do_all_flag,
                       file1.Global_Var_Names(), file2.Global_Var_Names(),
		       diff_found);

  // Build (and compare) nodal variable names.
  build_variable_names("nodal", interface.node_var_names, interface.node_var,
                       interface.node_var_default, interface.node_var_do_all_flag,
                       file1.Nodal_Var_Names(),  file2.Nodal_Var_Names(),
		       diff_found);

  // Build (and compare) element variable names.
  build_variable_names("element", interface.elmt_var_names, interface.elmt_var,
                        interface.elmt_var_default, interface.elmt_var_do_all_flag,
                        file1.Elmt_Var_Names(),   file2.Elmt_Var_Names(),
			diff_found);

  // Build (and compare) element variable names.
  if (!interface.ignore_attributes) {
    build_variable_names("element attribute", interface.elmt_att_names, interface.elmt_att,
			 interface.elmt_att_default, interface.elmt_att_do_all_flag,
			 file1.Elmt_Att_Names(),   file2.Elmt_Att_Names(),
			 diff_found);
  }

  // Build (and compare) nodeset variable names.
  build_variable_names("nodeset", interface.ns_var_names, interface.ns_var,
                       interface.ns_var_default, interface.ns_var_do_all_flag,
                       file1.NS_Var_Names(), file2.NS_Var_Names(),
                       diff_found);

  // Build (and compare) sideset variable names.
  build_variable_names("sideset", interface.ss_var_names, interface.ss_var,
                       interface.ss_var_default, interface.ss_var_do_all_flag,
                       file1.SS_Var_Names(), file2.SS_Var_Names(),
                       diff_found);
}

template <typename INT>
int Create_File(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2,
                const string& diffile_name, bool *diff_found)
{
  // Multiple modes:
  // summary_flag == true   --> Single file, output summary and variable names, return
  // diffile_name == ""     --> Dual file, output summary, variable names, check compatability,
  // diffile_name != ""     --> Three files (2 in, 1 out)
  //                            create output file which is diff of input.
  //                            output summary, variable names, check compatability
  // quiet_flag == true     --> don't output summary information


  SMART_ASSERT(!interface.summary_flag);
  //========================================================================
  // From here on down, have two input files and possibly 1 output file...
  // Create output file.

  int out_file_id = -1;
  if (!diffile_name.empty()) {

    // Take minimum word size for output file.
    int iows = file1.IO_Word_Size() < file2.IO_Word_Size()
      ? file1.IO_Word_Size() : file2.IO_Word_Size();
    int compws = sizeof(double);

    int mode = EX_CLOBBER;
    if (sizeof(INT) == 8) {
      mode |= EX_ALL_INT64_DB;
      mode |= EX_ALL_INT64_API;
    }
    out_file_id = ex_create(diffile_name.c_str(), mode, &compws, &iows);
    SMART_ASSERT(out_file_id >= 0);
    ex_copy(file1.File_ID(), out_file_id);
  }

  if (!interface.quiet_flag) {
    if (out_file_id >= 0) {  // The files are to be differenced .. just list names.
      if (interface.coord_tol.type != IGNORE) {
	sprintf(buf, "Coordinates:  tol: %8g %s, floor: %8g",
		interface.coord_tol.value, interface.coord_tol.typestr(), interface.coord_tol.floor);
	std::cout << buf << std::endl;
      }
      else
	std::cout << "Locations of nodes will not be considered.\n";

      if (interface.time_tol.type != IGNORE) {
	sprintf(buf, "Time step values:  tol: %8g %s, floor: %8g",
		interface.time_tol.value,	interface.time_tol.typestr(), interface.time_tol.floor);
	std::cout << buf << std::endl;
      }
      else
	std::cout << "Time step time values will not be differenced.\n";

      output_diff_names("Global",  interface.glob_var_names);
      output_diff_names("Nodal",   interface.node_var_names);
      output_diff_names("Element", interface.elmt_var_names);
      output_diff_names("Element Attribute", interface.elmt_att_names);
      output_diff_names("Nodeset", interface.ns_var_names);
      output_diff_names("Sideset", interface.ss_var_names);
    }
    else {  // The files are to be compared .. echo additional info.
      if (Tolerance::use_old_floor) {
	std::cout << "WARNING: Using old definition of floor tolerance. |a-b|<floor.\n\n";
      }
      if (interface.coord_tol.type != IGNORE) {
	sprintf(buf, "Coordinates will be compared .. tol: %8g (%s), floor: %8g",
		interface.coord_tol.value, interface.coord_tol.typestr(), interface.coord_tol.floor);
	std::cout << buf << std::endl;
      } else {
	std::cout << "Locations of nodes will not be compared." << std::endl;
      }

      if (interface.time_tol.type != IGNORE) {
	sprintf(buf, "Time step values will be compared .. tol: %8g (%s), floor: %8g",
		interface.time_tol.value, interface.time_tol.typestr(), interface.time_tol.floor);
	std::cout << buf << std::endl;
      } else {
	std::cout << "Time step time values will not be compared." << std::endl;
      }

      output_compare_names("Global",  interface.glob_var_names, interface.glob_var,
			   file1.Num_Global_Vars(), file2.Num_Global_Vars());

      output_compare_names("Nodal",   interface.node_var_names, interface.node_var,
			   file1.Num_Nodal_Vars(), file2.Num_Nodal_Vars());

      output_compare_names("Element", interface.elmt_var_names, interface.elmt_var,
			   file1.Num_Elmt_Vars(), file2.Num_Elmt_Vars());

      output_compare_names("Element Attribute", interface.elmt_att_names, interface.elmt_att,
			   file1.Num_Elmt_Atts(), file2.Num_Elmt_Atts());

      output_compare_names("Nodeset", interface.ns_var_names, interface.ns_var,
			   file1.Num_NS_Vars(), file2.Num_NS_Vars());

      output_compare_names("Sideset", interface.ss_var_names, interface.ss_var,
			   file1.Num_SS_Vars(), file2.Num_SS_Vars());
    }
  }

  std::vector<int> truth_tab;
  build_truth_table(EX_ELEM_BLOCK, "Element Block", interface.elmt_var_names, file1.Num_Elmt_Blocks(),
		    file1, file2, file1.Elmt_Var_Names(), file2.Elmt_Var_Names(),
		    truth_tab, interface.quiet_flag, diff_found);

  std::vector<int> ns_truth_tab;
  build_truth_table(EX_NODE_SET, "Nodeset", interface.ns_var_names, file1.Num_Node_Sets(),
		    file1, file2, file1.NS_Var_Names(), file2.NS_Var_Names(),
		    ns_truth_tab, interface.quiet_flag, diff_found);

  std::vector<int> ss_truth_tab;
  build_truth_table(EX_SIDE_SET, "Sideset", interface.ss_var_names, file1.Num_Side_Sets(),
		    file1, file2, file1.SS_Var_Names(), file2.SS_Var_Names(),
		    ss_truth_tab, interface.quiet_flag, diff_found);


  // Put out the concatenated variable parameters here and then
  // put out the names....
  if (out_file_id >= 0) {
    ex_put_all_var_param(out_file_id,
			 interface.glob_var_names.size(),
			 interface.node_var_names.size(),
			 interface.elmt_var_names.size(), TOPTR(truth_tab),
			 interface.ns_var_names.size(),   TOPTR(ns_truth_tab),
			 interface.ss_var_names.size(),   TOPTR(ss_truth_tab));

    output_exodus_names(out_file_id, EX_GLOBAL,     interface.glob_var_names);
    output_exodus_names(out_file_id, EX_NODAL,      interface.node_var_names);
    output_exodus_names(out_file_id, EX_ELEM_BLOCK, interface.elmt_var_names);
    output_exodus_names(out_file_id, EX_NODE_SET,   interface.ns_var_names);
    output_exodus_names(out_file_id, EX_SIDE_SET,   interface.ss_var_names);
  }
  return out_file_id;
}


namespace {
  void output_exodus_names(int file_id, EXOTYPE type, const vector<string> &names)
  {
    std::vector<char*> vars(names.size());
    if (names.size() > 0) {
      for (unsigned i = 0; i < names.size(); ++i) {
	vars[i] = (char*)(names[i].c_str());
	SMART_ASSERT(vars[i] != 0);
      }
      ex_put_variable_names(file_id, type, names.size(), TOPTR(vars));
    }
  }

  void output_compare_names(const char* type, const vector<string> &names, const std::vector<Tolerance> &tol,
			    int num_vars1, int num_vars2)
  {
    if (names.size() > 0) {
      std::cout << type << " variables to be compared:" << std::endl;
      for (unsigned v = 0; v < names.size(); ++v)
	{
	  if (v == 0)
	    sprintf(buf, "%-32s tol: %8g (%s), floor: %8g",
		    names[v].c_str(),
		    tol[v].value, tol[v].typestr(), tol[v].floor);
	  else
	    sprintf(buf, "%-32s      %8g (%s),        %8g",
		    names[v].c_str(),
		    tol[v].value, tol[v].typestr(), tol[v].floor);
	  std::cout << "\t" << buf << std::endl;
	}
    } else if (num_vars1 == 0 && num_vars2 == 0) {
      std::cout << "No " << type << " variables on either file.\n";
    } else {
      std::cout << type << " variables will not be compared.\n";
    }
  }

  void output_diff_names(const char *type, const vector<string> &names)
  {
    if (names.size() > 0) {
      std::cout << type << " variables to be differenced:" << std::endl;
      for (unsigned v = 0; v < names.size(); ++v)
	std::cout << "\t" << names[v] << std::endl;
    }
    else
      std::cout << "No " << type << " variables will be differenced." << std::endl;
  }

  void build_variable_names(const char *type, vector<string> &names, std::vector<Tolerance> &tols,
			    const Tolerance& default_tol, bool do_all_flag,
			    const vector<string> &var_names1, const vector<string> &var_names2,
			    bool *diff_found)
  {
    vector<string> x_list;  // exclusion list
    for (unsigned m = 0; m < names.size(); ++m) {
      string name = names[m];  chop_whitespace(name);
      SMART_ASSERT(name.size() >= 1);
      if (name[0] == '!')
	x_list.push_back( extract_token(name, "!") ); // remove "!" & add
    }

    if (do_all_flag) {
      int n;
      int name_length = var_names1.size();
      for (n = 0; n < name_length; ++n) {
	const string& name = var_names1[n];
	if (!interface.summary_flag && find_string(var_names2, name, interface.nocase_var_names) < 0) {
	  if (find_string(x_list, name, interface.nocase_var_names) < 0) {
	    if (interface.allowNameMismatch) {
	      x_list.push_back(name);
	    } else {
	      *diff_found = true;
	      if (!interface.quiet_flag)
		std::cout << "exodiff: WARNING .. " << type << " variable \"" << name
			  << "\" is in the first file but not the second." << std::endl;
	      continue;
	    }
	  }
	}
	if (find_string(names, name, interface.nocase_var_names) < 0 &&
	    find_string(x_list, name, interface.nocase_var_names) < 0) {
	  int idx = names.size();
	  names.push_back(name);
	  tols[idx]  = default_tol;
	}
      }

      if (!interface.noSymmetricNameCheck) {
	name_length = var_names2.size();
	for (n = 0; n < name_length; ++n) {
	  const string& name = var_names2[n];
	  if (!interface.summary_flag && find_string(var_names1, name, interface.nocase_var_names) < 0) {
	    if (find_string(x_list, name, interface.nocase_var_names) < 0 ) {
	      *diff_found = true;
	      if (!interface.quiet_flag)
		std::cout << "exodiff: WARNING .. " << type << " variable \"" << name
			  << "\" is in the second file but not the first." << std::endl;
	      continue;
	    }
	  }
	  SMART_ASSERT( find_string(names, name, interface.nocase_var_names) >= 0 ||
			find_string(x_list, name, interface.nocase_var_names) >= 0 );
	}
      }
    }

    vector<string> tmp_list;
    for (unsigned n = 0; n < names.size(); ++n) {
      string name = names[n];  chop_whitespace(name);
      if (name[0] == '!') continue;

      if (find_string(var_names1, name, interface.nocase_var_names) >= 0) {
	if (interface.summary_flag || find_string(var_names2, name, interface.nocase_var_names) >= 0)
	  {
	    tols[tmp_list.size()] = tols[n];
	    int idx = find_string(var_names1, name, interface.nocase_var_names);
	    tmp_list.push_back( var_names1[idx] );
	  }
	else {
	  *diff_found = true;
	  if (!interface.quiet_flag)
	    std::cout << "exodiff: WARNING .. " << type << " variable \"" << name
		      << "\" is not in the second file." << std::endl;
	}
      } else {
	*diff_found = true;
	if (!interface.quiet_flag)
	  std::cout << "exodiff: WARNING .. specified " << type << " variable \"" << name
		    << "\" is not in the first file." << std::endl;
      }
    }
    names = tmp_list;
  }

  template <typename INT>
  void build_truth_table(EXOTYPE type, const char *label, vector<string> &names, size_t num_entity,
			 ExoII_Read<INT>& file1, ExoII_Read<INT>& file2,
			 const vector<string> &var_names1, const vector<string> &var_names2,
			 std::vector<int> &truth_tab, bool quiet_flag, bool *diff_found)
  {
    if (names.size() > 0)	{
      int num_vars = names.size();

      truth_tab.resize(num_vars * num_entity);
      for (int i = num_vars * num_entity - 1; i >= 0; --i)
	truth_tab[i] = 0;

      for (size_t b = 0; b < num_entity; ++b) {
	Exo_Entity *set1 = file1.Get_Entity_by_Index(type, b);
	Exo_Entity *set2 = file2.Get_Entity_by_Id(type, set1->Id());
	if (set2 == NULL) {
	  *diff_found = true;
	  std::cout << "exodiff: WARNING " << label << " id " << set1->Id()
		    << " exists in first file but not the second...\n";
	  continue;
	}

	for (int out_idx = 0; out_idx < num_vars; ++out_idx) {
	  const string& name = names[out_idx];
	  int idx1 = find_string(var_names1, name, interface.nocase_var_names);
	  int idx2 = find_string(var_names2, name, interface.nocase_var_names);
	  if (set1->is_valid_var(idx1)) {
	    if (set2->is_valid_var(idx2))
	      truth_tab[ b * num_vars + out_idx ] = 1;
	    else if (!quiet_flag) {
	      std::cout << "exodiff: WARNING " << label << " variable \"" << name
			<< "\" is not saved for " << label << " id " << set1->Id()
			<< " in the second file but is "
			<< "in the first (by virtue of the truth tables).  "
			<< "This variable won't be considered for this " << label << ".\n";
	    }
	  }
	  else if (set2->is_valid_var(idx2) && !quiet_flag) {
	    std::cout << "exodiff: WARNING " << label << " variable \"" << name
		      << "\" is not saved for " << label << " id " << set1->Id()
		      << " in the first file but is "
		      << "in the second (by virtue of the truth tables).  "
		      << "This variable won't be considered for this " << label << ".\n";
	  }
	}
      }
    }
  }
} // End of namespace

template int Create_File(ExoII_Read<int>& file1, ExoII_Read<int>& file2,
			 const string& diffile_name, bool *diff_found);
template void Build_Variable_Names(ExoII_Read<int>& file1, ExoII_Read<int>& file2, bool *diff_found);

template int Create_File(ExoII_Read<int64_t>& file1, ExoII_Read<int64_t>& file2,
			 const string& diffile_name, bool *diff_found);
template void Build_Variable_Names(ExoII_Read<int64_t>& file1, ExoII_Read<int64_t>& file2, bool *diff_found);
