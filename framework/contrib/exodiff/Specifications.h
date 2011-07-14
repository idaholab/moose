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

#ifndef Options_h
#define Options_h

#include "Tolerance.h"
#include "map.h"
#include <string>
#include <vector>

#define DEFAULT_MAX_NUMBER_OF_NAMES 1000


class Specifications {
public:
  
  Specifications();
  ~Specifications();
  
  void Set_Max_Names(int size);
  void allocateNames();
  void allocateExcludeSteps( int num );
  
  // Program parameters.
  bool quiet_flag;        // By default, warnings and other info is produced
  bool show_all_diffs;    // Be default, show only maximum diff for each variable;
                          // if set, show all diff that exceeds tolerance.
  TOLERANCE_TYPE_enum output_type;  // By default, output file diffs are absolute.
  MAP_TYPE_enum map_flag;          // By default, no searching is done to match
                          // nodes & elements.
  bool nsmap_flag;        // By default, nodeset nodelist match is off
  bool ssmap_flag;        // By default, sideset elem/side match is off
  bool short_block_check; // By default, element block compares are
                          // case in-sensitive and full.  This switch
                          // checks only up to the shortest string length.
  bool nocase_var_names;  // By default, variable name compares are
                          // case sensitive and full.  This switch
                          // ignores case when comparing.
  bool summary_flag;      // By default, summary mode is not in effect.
  bool ignore_maps;       // By default, use the node and element number
                          // maps to report node and element ids.
  bool ignore_nans;       // Don't check for NaNs
  bool ignore_dups;       // If two elements/nodes in same location in map or partial map
                          // case, just return first match instead of aborting.

  bool ignore_attributes; // Don't compare attributes...

  bool coord_sep;
  bool exit_status_switch;   
  bool dump_mapping;         // By default, mappings are not printed.
  bool show_unmatched;       // Show elements not matched in partial mode
  bool noSymmetricNameCheck; // By default, the second file's variable
  bool allowNameMismatch;    // By default, name in 1st db must be in second also.
  bool doNorms;
  
  // These should correspond to the values specified during parsing of
  // coordinate tolerance.
  Tolerance coord_tol; 
  Tolerance time_tol;
  
  // Offset of timesteps between first and second databases.
  int time_step_offset; 
  int time_step_start; // First step to compare (1-based)
  int time_step_stop;  // Last step to compare
  int time_step_increment;  // Step increment
  
  int max_number_of_names;
  
  Tolerance default_tol;
  
  std::vector<std::string>* glob_var_names;
  bool                      glob_var_do_all_flag;
  Tolerance                 glob_var_default;
  Tolerance*                glob_var;
  
  std::vector<std::string>* node_var_names;
  bool                      node_var_do_all_flag;
  Tolerance                 node_var_default;
  Tolerance*                node_var;
  
  std::vector<std::string>* elmt_var_names;
  bool                      elmt_var_do_all_flag;
  Tolerance                 elmt_var_default;
  Tolerance*                elmt_var;
  
  std::vector<std::string>* elmt_att_names;
  bool                      elmt_att_do_all_flag;
  Tolerance                 elmt_att_default;
  Tolerance*                elmt_att;
  
  std::vector<std::string>* ns_var_names;
  bool                      ns_var_do_all_flag;
  Tolerance                 ns_var_default;
  Tolerance*                ns_var;
  
  std::vector<std::string>* ss_var_names;
  bool                      ss_var_do_all_flag;
  Tolerance                 ss_var_default;
  Tolerance*                ss_var;
  
  // time step exclusion data
  int  num_excluded_steps;
  int* exclude_steps;

  std::string command_file_name;
  
 private:
  Specifications(const Specifications&); // don't implement
  Specifications& operator=(const Specifications&); // don't implement
};

extern Specifications specs;

#endif
