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
#ifndef Sierra_SystemInterface_h
#define Sierra_SystemInterface_h

#include "GetLongOpt.h" // for GetLongOption
#include "Tolerance.h"  // for Tolerance, etc
#include "terminal_color.h"
#include "util.h"
#include <cmath>
#include <string>  // for string
#include <utility> // for pair
#include <vector>  // for vector

#define DEFAULT_MAX_NUMBER_OF_NAMES 1000

#define ERROR(x)                                                                                   \
  do                                                                                               \
  {                                                                                                \
    std::ostringstream out;                                                                        \
    out << "exodiff: ERROR: " << x;                                                                \
    ERR_OUT(out);                                                                                  \
  } while (0)

class SystemInterface
{
public:
  SystemInterface();
  ~SystemInterface();

  bool parse_options(int argc, char **argv);

  static void show_version();

  void Parse_Command_File();
  void Set_Max_Names(int size);

  // Program parameters.
  Tolerance coord_tol{ABSOLUTE, 1.0e-6, 0.0};
  Tolerance time_tol{RELATIVE, 1.0e-6, 1.0e-15};
  Tolerance final_time_tol{RELATIVE, 0.0, 0.0};
  Tolerance default_tol{RELATIVE, 1.0e-6, 0.0};
  Tolerance ss_df_tol{RELATIVE, 1.0e-6, 0.0};

  // These should correspond to the values specified during parsing of
  // coordinate tolerance.
  // Offset of timesteps between first and second databases.
  int time_step_offset{0};
  int time_step_start{1};     // First step to compare (1-based)
  int time_step_stop{-1};     // Last step to compare
  int time_step_increment{1}; // Step increment

  std::pair<int, int> explicit_steps; // Only compare these two steps (db1:db2) if nonzero.

  int max_number_of_names{DEFAULT_MAX_NUMBER_OF_NAMES};

  std::vector<std::string> glob_var_names;
  Tolerance glob_var_default{RELATIVE, 1.0e-6, 0.0};
  std::vector<Tolerance> glob_var;

  std::vector<std::string> node_var_names;
  Tolerance node_var_default{RELATIVE, 1.0e-6, 0.0};
  std::vector<Tolerance> node_var;

  std::vector<std::string> elmt_var_names;
  Tolerance elmt_var_default{RELATIVE, 1.0e-6, 0.0};
  std::vector<Tolerance> elmt_var;

  std::vector<std::string> elmt_att_names;
  Tolerance elmt_att_default{RELATIVE, 1.0e-6, 0.0};
  std::vector<Tolerance> elmt_att;

  std::vector<std::string> ns_var_names;
  Tolerance ns_var_default{RELATIVE, 1.0e-6, 0.0};
  std::vector<Tolerance> ns_var;

  std::vector<std::string> ss_var_names;
  Tolerance ss_var_default{RELATIVE, 1.0e-6, 0.0};
  std::vector<Tolerance> ss_var;

  // time step exclusion data
  std::vector<int> exclude_steps;

  std::string file1;
  std::string file2;
  std::string diff_file;
  std::string command_file;

  bool quiet_flag{false};     // By default, warnings and other info is produced
  bool show_all_diffs{false}; // Be default, show only maximum diff for each variable;
                              // if set, show all diff that exceeds tolerance.
  TOLERANCE_TYPE_enum output_type{ABSOLUTE}; // By default, output file diffs are absolute.
  MAP_TYPE_enum map_flag{USE_FILE_IDS};      // By default, no searching is done to match
                                             // nodes & elements.
  bool nsmap_flag{true};                     // By default, nodeset nodelist match is off
  bool ssmap_flag{true};                     // By default, sideset elem/side match is off
  bool short_block_check{true};              // By default, element block compares are
                                             // case in-sensitive and full.  This switch
                                             // checks only up to the shortest string length.
  bool nocase_var_names{true};               // By default, variable name compares are
                                             // case sensitive and full.  This switch
                                             // ignores case when comparing.
  bool summary_flag{false};                  // By default, summary mode is not in effect.
  bool ignore_maps{false};                   // By default, use the node and element number
                                             // maps to report node and element ids.
  bool ignore_nans{false};                   // Don't check for NaNs
  bool ignore_dups{false}; // If two elements/nodes in same location in map or partial map
                           // case, just return first match instead of aborting.

  bool ignore_attributes{false}; // Don't compare attributes...
  bool ignore_sideset_df{false}; // Don't compare sideset df

  bool ints_64_bits{false};

  bool coord_sep{false};
  bool exit_status_switch{true};
  bool dump_mapping{false};         // By default, mappings are not printed.
  bool show_unmatched{false};       // Show elements not matched in partial mode
  bool noSymmetricNameCheck{false}; // By default, the second file's variable
  bool allowNameMismatch{false};    // By default, name in 1st db must be in second also.
  bool doL1Norm{false};
  bool doL2Norm{false};
  bool pedantic{false}; // Be most picky on what is different (not fully picky yet)

  bool interpolating{false}; // Interpolate times on file2 to match times on file1;
  bool by_name{false};       // Match entities by name instead of by id.

  bool glob_var_do_all_flag{false};
  bool node_var_do_all_flag{false};
  bool elmt_var_do_all_flag{false};
  bool elmt_att_do_all_flag{false};
  bool ns_var_do_all_flag{false};
  bool ss_var_do_all_flag{false};

private:
  void enroll_options();
  GetLongOption options_; //!< Options parsing
};

extern SystemInterface interface;
#endif
