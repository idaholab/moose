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
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#ifndef PRId64
#error "PRId64 not defined"
#endif

#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <float.h>

#include "exodiff.h"
#include "Tolerance.h"
#include "MinMaxData.h"
#include "map.h"
#include "smart_assert.h"
#include "exoII_read.h"
#include "exo_block.h"
#include "node_set.h"
#include "side_set.h"
#include "libmesh/exodusII.h"
#include "stringx.h"
#include "FileInfo.h"
#include "ED_SystemInterface.h"
#include "ED_Version.h"

#include "add_to_log.h"

using namespace std;

SystemInterface interface;

struct TimeInterp
{
  TimeInterp() :
    step1(-1), step2(-1), time(0.0), proportion(1.0) {}

  int step1; // step at beginning of interval. -1 if time prior to time at step1
  int step2; // step at end of interval. -1 if time after time at step2

  double time; // Time being interpolated to.

  // If t1 = time at step1 and t2 = time at step2,
  // then proportion = (time-t2)/(t2-t1)
  // Or, value at time = proportion*v1 + (1.0-proportion)*v2
  double proportion;
}
;

string Date() {
  char tbuf[32];
  time_t calendar_time = time(NULL);
  struct tm *local_time = localtime(&calendar_time);
  strftime(tbuf, 32, "%Y/%m/%d   %H:%M:%S %Z", local_time);
  string time_string(tbuf);
  return time_string;
}

bool Invalid_Values(const double *values, size_t count);

void Print_Banner(const char *prefix)
{
  std::cout
  << "\n"
  << prefix << "  *****************************************************************\n"
  << prefix << "             ";
  SystemInterface::show_version();
  std::cout
  << prefix <<  "             Authors:  Richard Drake, rrdrake@sandia.gov           \n"
  << prefix <<  "                       Greg Sjaardema, gdsjaar@sandia.gov          \n"
  << prefix <<  "             Run on    " << Date()                           << "\n"
  << prefix <<  "  *****************************************************************\n"
  << std::endl;
}

  // TODO:  - copy node & side sets

  //        - copy node & element maps
  //        - copy coordinate variable names (instead of always using X,Y,Z)
  //        - more checks on success of Exodus calls

  // Issues: - When mapping element numbers, blocks are irrelevant.  Problem is
  //           the variables that are determined to be stored in each file are
  //           NOT independent of blocks .. in fact, that is how it determines
  //           if the two files have the same element variable stored.  The
  //           mapping will still run ok, just don't expect it to work if the
  //           blocks don't line up and different variables are stored in
  //           different blocks.


  template <typename INT>
  extern void Build_Variable_Names(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, bool *diff_found);

  template <typename INT>
  extern bool Check_Global( ExoII_Read<INT>& file1, ExoII_Read<INT>& file2);

  template <typename INT>
  extern void Check_Compatible_Meshes( ExoII_Read<INT>& file1,
				       ExoII_Read<INT>& file2,
				       bool check_only,
				       const INT *node_map,
				       const INT *elmt_map,
				       const INT *node_id_map);

  template <typename INT>
  int Create_File(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2,
		  const string& diffile_name, bool *diff_found);

  double To_Double(const string & str_val);

  double FileDiff(double v1, double v2, TOLERANCE_TYPE_enum type);

  void Die_TS(double ts);

  template <typename INT>
  size_t global_elmt_num(ExoII_Read<INT>& file, size_t b_idx, size_t e_idx);

  template <typename INT>
  double Find_Min_Coord_Sep(ExoII_Read<INT>& file);

  int timeStepIsExcluded(int ts);

  template <typename INT>
  const double *get_nodal_values( ExoII_Read<INT> &filen, int time_step,
				  size_t idx, size_t fno, const string &name, bool *diff_flag );
  template <typename INT>
  const double *get_nodal_values( ExoII_Read<INT> &filen, const TimeInterp &t,
				  size_t idx, size_t fno, const string &name, bool *diff_flag );

  template <typename INT>
  void do_diffs(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int time_step1, TimeInterp t2, int out_file_id,
		MinMaxData *mm_glob, MinMaxData *mm_node, MinMaxData *mm_elmt,
		MinMaxData *mm_ns, MinMaxData *mm_ss,
		INT *node_map, const INT *node_id_map, INT *elmt_map, const INT *elem_id_map,
		Exo_Block<INT> **blocks2, double *var_vals, bool *diff_flag);

  template <typename INT>
  bool diff_globals( ExoII_Read<INT>& file1, ExoII_Read<INT>& file2,
		     int step1, TimeInterp step2, int out_file_id,
		     MinMaxData *mm_glob, double *vals );
  template <typename INT>
  bool diff_nodals( ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp step2,
		    int out_file_id, INT* node_map, const INT* id_map,
		    MinMaxData *mm_node, double *vals);
  template <typename INT>
  bool diff_element( ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp step2,
		     int out_file_id, INT* elmt_map, const INT *id_map,
		     Exo_Block<INT> **blocks2, MinMaxData *mm_elmt,
		     double *vals );

  template <typename INT>
  bool diff_element_attributes( ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, INT* elmt_map, const INT *id_map,
				Exo_Block<INT> **blocks2);

  template <typename INT>
  bool diff_nodeset( ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp step2,
		     int out_file_id, const INT *id_map, MinMaxData *mm_ns,
		     double *vals );

  template <typename INT>
  bool diff_sideset( ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp step2,
		     int out_file_id, const INT *id_map, MinMaxData *mm_ss,
		     double *vals );

  template <typename INT>
  void output_summary( ExoII_Read<INT>& file1, MinMaxData &mm_time,
		       MinMaxData *mm_glob, MinMaxData *mm_node,
		       MinMaxData *mm_elmt, MinMaxData *mm_ns, MinMaxData *mm_ss,
		       const INT *node_id_map, const INT *elem_id_map );


#include <signal.h>
  // bit of a hack to get GNU's functions to enable floating point error trapping
#ifdef LINUX
#ifdef __USE_GNU
#include <fenv.h>
#else
#define __USE_GNU
#include <fenv.h>
#undef __USE_GNU
#endif
#endif

  struct sigaction sigact;    // the signal handler & blocked signals
  bool checking_invalid = false;
  bool invalid_data = false;
  extern "C" {
    void floating_point_exception_handler(int signo)
    {
      if (!checking_invalid) {
	std::cout << "exodiff: caught floating point exception (" << signo << ")"
		  << " bad data?" << std::endl;
	exit(1);
      }
      else
	invalid_data = true;
    }
  }

namespace {
  int get_int_size(const std::string &file_name)
  {
    if (file_name == "")
      return 0;

    int ws = 0, comp_ws = 8;
    float dum = 0.0;
    int err = ex_open(file_name.c_str(), EX_READ,
		      &comp_ws, &ws, &dum);
    if (err < 0) {
      std::cerr << "ERROR: Couldn't open file \"" << file_name << "\".";
      return 0;
    }
    if (ex_int64_status(err) & EX_ALL_INT64_DB)
      return 8;
    else
      return 4;
  }

  template <typename INT>
  TimeInterp get_surrounding_times(double time, ExoII_Read<INT>& file)
  {
    TimeInterp tprop;
    tprop.time = time;

    int num_times = file.Num_Times();
    if (num_times == 0 || time < file.Time(1)) {
      tprop.step2 = 0;
      return tprop;
    }

    if (time > file.Time(num_times)) {
      tprop.step1 = 0;
      return tprop;
    }

    int tbef = 1;
    for (int i=2; i <= num_times; i++) {
      if (file.Time(i) <= time)
	tbef = i;
      else if (interface.time_tol.type != IGNORE && !interface.time_tol.Diff(time, file.Time(i)))
	tbef = i;
      else
	break;
    }

    if (!interface.time_tol.Diff(time, file.Time(tbef))) {
      tprop.step1 = tprop.step2 = tbef;
      return tprop;
    } else {
      SMART_ASSERT(tbef+1 <= num_times)(tbef+1)(num_times);
      tprop.step1 = tbef;
      tprop.step2 = tbef+1;

      // Calculate proprtion...
      double t1 = file.Time(tbef);
      double t2 = file.Time(tbef+1);
      tprop.proportion = (t2 - time) / (t2 - t1);
      return tprop;
    }
  }

  template <typename INT>
  void output_init(ExoII_Read<INT>& file, int count, const char *prefix)
  {
    FileInfo fi(file.File_Name());
    std::cout << prefix << "  FILE " << count << ": " << fi.realpath() << std::endl
	      << prefix << "   Title: " << file.Title() << std::endl
	      << prefix << "          Dim = " << file.Dimension()
	      << ", Blocks = " << file.Num_Elmt_Blocks()
	      << ", Nodes = " << file.Num_Nodes()
	      << ", Elements = " << file.Num_Elmts()
	      << ", Nodesets = " << file.Num_Node_Sets()
	      << ", Sidesets = " << file.Num_Side_Sets() << std::endl
	      << prefix << "          Vars: Global = " << file.Num_Global_Vars()
	      << ", Nodal = " << file.Num_Nodal_Vars()
	      << ", Element = " << file.Num_Elmt_Vars()
	      << ", Nodeset = " << file.Num_NS_Vars()
	      << ", Sideset = " << file.Num_SS_Vars()
	      << ", Times = " << file.Num_Times()
	      << "\n\n";
  }

  char buf[256];

  template <typename INT>
  bool exodiff(ExoII_Read<INT> &file1, ExoII_Read<INT> &file2);
}


  int main(int argc, char* argv[])
  {
    interface.Set_Max_Names(DEFAULT_MAX_NUMBER_OF_NAMES);
    bool ok = interface.parse_options(argc, argv);

    if (!ok)
      exit(1);

    checking_invalid = false;
    invalid_data = false;

    if (sigfillset(&(sigact.sa_mask)) == -1) perror("sigfillset failed");
    sigact.sa_handler = floating_point_exception_handler;
    if (sigaction(SIGFPE, &sigact, 0) == -1) perror("sigaction failed");
#if defined(LINUX) && defined(GNU)
    // for GNU, this seems to be needed to turn on trapping
    feenableexcept(FE_DIVBYZERO | FE_OVERFLOW | FE_INVALID);
#endif

    string file1_name    = interface.file1;
    string file2_name    = interface.file2;
    string diffile_name  = interface.diff_file;

    if (interface.summary_flag && file1_name == "") {
      std::cout << "exodiff: ERROR: Summary option engaged but an exodus "
	"file was not specified" << std::endl;
      exit(1);
    }

    if (interface.summary_flag) {
      file2_name = "";
      diffile_name = "";
      interface.glob_var_do_all_flag = true;
      interface.node_var_do_all_flag = true;
      interface.elmt_var_do_all_flag = true;
      interface.elmt_att_do_all_flag = true;
      interface.ns_var_do_all_flag   = true;
      interface.ss_var_do_all_flag   = true;
      interface.map_flag             = FILE_ORDER;
      interface.quiet_flag           = false;
    }

    if (!interface.quiet_flag && !interface.summary_flag) Print_Banner(" ");
    if (interface.summary_flag) Print_Banner("#");

    // Check integer sizes in input file(s)...
    int int_size = 4;
    if (interface.ints_64_bits) {
      int_size = 8;
    }
    else if (get_int_size(file1_name) == 8) {
      int_size = 8;
    }
    else if (!interface.summary_flag && get_int_size(file2_name) == 8) {
      int_size = 8;
    }

    bool diff_flag = true;
    if (int_size == 4) {
      // Open input files.
      ExoII_Read<int> file1(file1_name.c_str());
      ExoII_Read<int> file2(file2_name.c_str());
      diff_flag = exodiff(file1, file2);
    } else {
      // Open input files.
      ExoII_Read<int64_t> file1(file1_name.c_str());
      ExoII_Read<int64_t> file2(file2_name.c_str());
      diff_flag = exodiff(file1, file2);
    }
#if 0
    add_to_log(argv[0], 0);
#else
    // Temporarily differentiate this version from previous version in logs.
    std::string code = "exodiff-" + version;
    add_to_log( code.c_str(), 0 );
#endif

    if (interface.exit_status_switch && diff_flag)
      return 2;
    else
      return 0;
  }

namespace {
  template <typename INT>
  bool exodiff(ExoII_Read<INT> &file1, ExoII_Read<INT> &file2)
  {
    if (!interface.quiet_flag && !interface.summary_flag)
      std::cout << "Reading first file ... " << std::endl;
    string serr = file1.Open_File();
    if (!serr.empty()) {
      std::cout << "exodiff: " << serr << std::endl;
      exit(1);
    }
    if (!interface.summary_flag) {
      if (!interface.quiet_flag) std::cout << "Reading second file ... " << std::endl;
      serr = file2.Open_File();
      if (serr != "") {
	std::cout << "exodiff: " << serr << std::endl;
	exit(1);
      }
    }

    // Check that the maximum number of names has not been exceeded...
    if (file1.Num_Global_Vars() > interface.max_number_of_names ||
	file1.Num_Nodal_Vars()  > interface.max_number_of_names ||
	file1.Num_NS_Vars()     > interface.max_number_of_names ||
	file1.Num_SS_Vars()     > interface.max_number_of_names ||
	file1.Num_Elmt_Vars()   > interface.max_number_of_names) {
      int max = file1.Num_Global_Vars();
      if (file1.Num_Nodal_Vars() > max) max = file1.Num_Nodal_Vars();
      if (file1.Num_NS_Vars()    > max) max = file1.Num_NS_Vars();
      if (file1.Num_SS_Vars()    > max) max = file1.Num_SS_Vars();
      if (file1.Num_Elmt_Vars()  > max) max = file1.Num_Elmt_Vars();

      std::cout << "exodiff: Number of names in file 1 (" << max << ") is larger than "
		<< "current limit of " << interface.max_number_of_names
		<< ".  To increase, use \"-maxnames <int>\" on the command "
	"line or \"MAX NAMES <int>\" in the command file.  "
	"Aborting..."
		<< std::endl;
      exit(1);
    }

    // Check that the maximum number of names has not been exceeded...
    if (!interface.summary_flag) {
      if (file2.Num_Global_Vars() > interface.max_number_of_names ||
	  file2.Num_Nodal_Vars()  > interface.max_number_of_names ||
	  file2.Num_NS_Vars()     > interface.max_number_of_names ||
	  file2.Num_SS_Vars()     > interface.max_number_of_names ||
	  file2.Num_Elmt_Vars()   > interface.max_number_of_names) {
	int max = file2.Num_Global_Vars();
	if (file2.Num_Nodal_Vars() > max) max = file2.Num_Nodal_Vars();
	if (file2.Num_NS_Vars()    > max) max = file2.Num_NS_Vars();
	if (file2.Num_SS_Vars()    > max) max = file2.Num_SS_Vars();
	if (file2.Num_Elmt_Vars()  > max) max = file2.Num_Elmt_Vars();

	std::cout << "exodiff: Number of names in file 2 (" << max << ") is larger than "
		  << "current limit of " << interface.max_number_of_names
		  << ".  To increase, use \"-maxnames <int>\" on the command "
	  "line or \"MAX NAMES <int>\" in the command file.  "
	  "Aborting..."
		  << std::endl;
	exit(1);
      }
    }

    if (interface.summary_flag) {
      output_init(file1, 1, "#");
    } else {
      if (!interface.quiet_flag) {
	output_init(file1, 1, "");
	output_init(file2, 2, "");
	if (!interface.command_file.empty()) {
	  FileInfo fi(interface.command_file);
	  cout << "  COMMAND FILE: " << fi.realpath() << "\n\n";
	}
      }
    }

    if (!interface.summary_flag) {
      bool is_same = Check_Global(file1, file2);
      if (!is_same) {
	file1.Close_File();
	file2.Close_File();
	std::cout << "\nexodiff: Files are different" << std::endl;
	if (interface.exit_status_switch)
	  return 2;
	else
	  return 0;
      }
    }

    // When mapping is on ("-m"), node_map maps indexes from file1 to indexes
    // into file2.  Similarly with elmt_map.
    INT *node_map = 0;
    INT *elmt_map = 0;
    if (interface.map_flag != FILE_ORDER) {
      if(interface.map_flag == PARTIAL) {
	Compute_Partial_Maps(node_map, elmt_map, file1, file2);
      } else if (interface.map_flag == USE_FILE_IDS) {
	Compute_FileId_Maps(node_map, elmt_map, file1, file2);
      } else if (interface.map_flag == DISTANCE) {
	Compute_Maps(node_map, elmt_map, file1, file2);
      } else {
	std::cout << "INTERNAL ERROR: Invalid map option." << std::endl;
      }

      if (interface.dump_mapping) {
	Dump_Maps(node_map, elmt_map, file1);
      }
      if (Check_Maps(node_map, elmt_map, file1, file2)) {
	if (interface.map_flag == DISTANCE) {
	  std::cout << "exodiff: INFO .. Map option is not needed.\n";
	}
	interface.map_flag = FILE_ORDER;
      }
    }

    bool diff_flag = false; // Set to 'true' to indicate files contain diffs
    // Call this before checking for compatible meshes since it sets which variables
    // are going to be compared.  If no variables of a specific type, then not an error
    // if the meshes are different in that type.
    Build_Variable_Names(file1, file2, &diff_flag);

    // Get node and element number maps which map internal implicit ids into
    // global ids...
    // For now, assume that both files have the same map. At some point, need
    // to actually use the maps to build the correspondence map from one file
    // to the next...
    const INT *node_id_map = NULL;
    const INT *elem_id_map = NULL;
    if (!interface.ignore_maps) {
      file1.Load_Node_Map();
      file1.Load_Elmt_Map();
      node_id_map = file1.Get_Node_Map();
      elem_id_map = file1.Get_Elmt_Map();
      if (!interface.summary_flag)
	Compare_Maps(file1, file2, node_map, elmt_map, interface.map_flag == PARTIAL);
    } else {
      node_id_map = new INT[file1.Num_Nodes()];
      INT *tmp_map = const_cast<INT*>(node_id_map);
      for (size_t i=0; i < file1.Num_Nodes(); i++) {
	tmp_map[i] = i+1;
      }
      elem_id_map = new INT[file1.Num_Elmts()];
      tmp_map = const_cast<INT*>(elem_id_map);
      for (size_t i=0; i < file1.Num_Elmts(); i++) {
	tmp_map[i] = i+1;
      }
    }

    int out_file_id = -1;
    if (!interface.summary_flag) {
      string diffile_name  = interface.diff_file;
      Check_Compatible_Meshes( file1, file2, (diffile_name == ""),
			       node_map, elmt_map, node_id_map );
      // Doesn't return if meshes are not compatible...

      out_file_id = Create_File(file1, file2, diffile_name, &diff_flag);
    }

    SMART_ASSERT( !( interface.summary_flag && out_file_id >= 0 ) );

    if (!interface.quiet_flag && !interface.summary_flag) {
      std::cout << "\n  ==============================================================\n";
      if (!interface.ignore_maps)
	std::cout << "  NOTE: All node and element ids are reported as global ids.\n\n";
      else
	std::cout << "  NOTE: All node and element ids are reported as local ids.\n\n";
      if (interface.interpolating)
	std::cout << "  NOTE: Interpolation mode is enabled.\n\n";
    }
    else if (interface.summary_flag) {
      std::cout << "\n# ==============================================================\n";
      if (!interface.ignore_maps)
	std::cout << "#  NOTE: All node and element ids are reported as global ids.\n\n";
      else
	std::cout << "#  NOTE: All node and element ids are reported as local ids.\n\n";
    }

    double* var_vals = 0;
    if (out_file_id >= 0) {
      size_t max_ent = interface.glob_var_names.size();
      if (file1.Num_Nodes() > max_ent)
	max_ent = file1.Num_Nodes();
      if (file1.Num_Elmts() > max_ent)
	max_ent = file1.Num_Elmts();

      var_vals = new double[max_ent];
    }

    // When mapping is in effect, it is efficient to grab pointers to all blocks.
    Exo_Block<INT>** blocks2 = 0;
    if (elmt_map != 0) {
      blocks2 = new Exo_Block<INT>*[file2.Num_Elmt_Blocks()];
      for (int b = 0; b < file2.Num_Elmt_Blocks(); ++b)
	blocks2[b] = file2.Get_Elmt_Block_by_Index(b);
    }

    // Diff attributes...
    if (!interface.ignore_attributes && elmt_map==NULL && !interface.summary_flag) {
      if (diff_element_attributes(file1, file2, elmt_map, elem_id_map, blocks2))
	diff_flag = true;
    }

    int min_num_times = file1.Num_Times();

    MinMaxData mm_time;   mm_time.type = MinMaxData::mm_time;
    MinMaxData *mm_glob = NULL;
    MinMaxData *mm_node = NULL;
    MinMaxData *mm_elmt = NULL;
    MinMaxData *mm_eatt = NULL;
    MinMaxData *mm_ns   = NULL;
    MinMaxData *mm_ss   = NULL;

    if (interface.summary_flag) {
      int n;
      if ((n = interface.glob_var_names.size()) > 0) {
	mm_glob = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_glob[i].type = MinMaxData::mm_global;
      }
      if ((n = interface.node_var_names.size()) > 0) {
	mm_node = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_node[i].type = MinMaxData::mm_nodal;
      }
      if ((n = interface.elmt_var_names.size()) > 0) {
	mm_elmt = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_elmt[i].type = MinMaxData::mm_element;
      }
      if ((n = interface.elmt_att_names.size()) > 0) {
	mm_eatt = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_eatt[i].type = MinMaxData::mm_elematt;
      }
      if ((n = interface.ns_var_names.size()) > 0) {
	mm_ns = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_ns[i].type = MinMaxData::mm_nodeset;
      }
      if ((n = interface.ss_var_names.size()) > 0) {
	mm_ss = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_ss[i].type = MinMaxData::mm_sideset;
      }
    }
    else {
      min_num_times = (file1.Num_Times() < file2.Num_Times()
		       ? file1.Num_Times() : file2.Num_Times());

      if (interface.interpolating) {
	min_num_times = file1.Num_Times();
      }

      if (interface.time_step_stop > 0 && interface.time_step_stop < min_num_times) {
	min_num_times = interface.time_step_stop;
      }
    }

    // If explicit times are set, then only want to diff a single time at those
    // specified times....
    if (interface.explicit_steps.first != 0 && interface.explicit_steps.second != 0) {
      int ts1 = interface.explicit_steps.first;
      if (ts1 == -1) {
	ts1 = file1.Num_Times();
      }
      int ts2 = interface.explicit_steps.second;
      if (ts2 == -1) {
	ts2 = file2.Num_Times();
      }
      TimeInterp t2;
      t2.step1 = ts2;
      t2.step2 = ts2;
      t2.time  = file2.Time(ts2);
      t2.proportion = 1.0;

      if (!interface.quiet_flag) {
	if (out_file_id >= 0)  {
	  std::cout << "Processing explicit time steps. File 1 step = " << ts1
		    << "  File 2 step = " << ts2 << std::endl;
	}
	else {
	  sprintf(buf, "  --------- Explicit Time step File 1: %d, %13.7e ~ File 2: %d, %13.7e ---------",
		  ts1, file1.Time(ts1), ts2, t2.time);
	  std::cout << buf << std::endl;
	}
      }

      do_diffs(file1, file2, ts1, t2, out_file_id,
	       mm_glob, mm_node, mm_elmt, mm_ns, mm_ss,
	       node_map, node_id_map, elmt_map, elem_id_map,
	       blocks2, var_vals, &diff_flag);
    }
    else {

      // If time_step_offset == -1, then determine the offset automatically.
      // Assumes file1 has more steps than file2 and that the last step(s)
      // on file2 match the last step(s) on file1.
      if (interface.time_step_offset == -1) {
	interface.time_step_offset = file1.Num_Times() - file2.Num_Times();
	if (interface.time_step_offset < 0) {
	  std::cout << "ERROR: Second database must have less timesteps than "
		    << "first database." << std::endl;
	  exit(1);
	}
      }

      // If time_step_offset == -2, then determine the offset automatically.
      // Find the closest time on file1 to the first time on file2.
      // Assumes file1 has more steps than file2.
      if (interface.time_step_offset == -2) {
	if (file1.Num_Times() < file2.Num_Times()) {
	  std::cout << "ERROR: Second database must have less timesteps than "
		    << "first database." << std::endl;
	  exit(1);
	}

	double t2 = file2.Time(1);
	double mindiff = fabs(t2 - file1.Time(1));
	int step = 1;
	for (int i=2; i < file1.Num_Times(); i++) {
	  double t1 = file1.Time(i);
	  double diff = fabs(t2 - t1);
	  if (diff < mindiff) {
	    step = i;
	    mindiff = diff;
	  }
	}
	interface.time_step_offset = step-1;
      }

      if (interface.time_step_offset > 0) {
	if (interface.time_step_start > 0) {
	  std::cout << "The first " << interface.time_step_offset+interface.time_step_start-1
		    << " timesteps in the first database will be skipped because of time step offset and time step start settings.\n\n";
	} else {
	  std::cout << "The first " << interface.time_step_offset
		    << " timesteps in the first database will be skipped because of time step offset setting.\n\n";
	}
      }

      if (interface.time_step_start == -1) {
	// Want to compare the last timestep on both databases...
	int time_step1 = file1.Num_Times();
	int time_step2 = file2.Num_Times();
	interface.time_step_start = time_step2;
	interface.time_step_offset = time_step1 - time_step2;
	min_num_times = interface.time_step_start;
	std::cout << "Comparing only the final step (step " << time_step1
		  << " on first, step " << time_step2 << " on second) on each database.\n\n";
      }
      else if (interface.time_step_start < 0) {
	interface.time_step_start = min_num_times;
      }
      else if (interface.time_step_start < 1) {
	interface.time_step_start = 1;
      }

      if (interface.time_step_start > min_num_times && min_num_times > 0) {
	std::cout << "\tERROR: Time step options resulted in no timesteps being compared\n";
	diff_flag = true;
      }

      for (int time_step = interface.time_step_start; time_step <= min_num_times;
	   time_step += interface.time_step_increment)
	{
	  if (timeStepIsExcluded(time_step)) continue;

	  int time_step1 = time_step + interface.time_step_offset;
	  int time_step2 = time_step;
	  SMART_ASSERT(time_step1 <= file1.Num_Times());

	  TimeInterp t2;
	  if (!interface.summary_flag) {
	    t2 = get_surrounding_times(file1.Time(time_step1), file2);
	    if (!interface.interpolating) {
	      t2.step1 = time_step2;
	      t2.step2 = time_step2;
	      t2.time  = file2.Time(time_step2);
	      t2.proportion = 1.0;
	    }
	    SMART_ASSERT(t2.step1 <= file2.Num_Times());
	    SMART_ASSERT(t2.step2 <= file2.Num_Times());
	  }

	  if (interface.summary_flag) {
	    double t = file1.Time(time_step1);
	    mm_time.spec_min_max(t, time_step1);
	  }
	  else if (out_file_id >= 0 && !interface.quiet_flag) {
	    std::cout << "Processing time step " << time_step1
		      << "  (Difference in time values = "
		      << (file1.Time(time_step1) - file2.Time(time_step2))
		      << ")" << std::endl;
	  }
	  else if (out_file_id < 0) {
	    if (!interface.quiet_flag) {
	      if (interface.interpolating) {
		if (t2.step1 == -1) {
		  sprintf(buf, "  --------- Time step %d, %13.7e ~ Skipping - Before all times on file2 (INTERPOLATING)",
			  time_step1,
			  file1.Time(time_step1));
		} else if (t2.step2 == -1) {
		  sprintf(buf, "  --------- Time step %d, %13.7e ~ Skipping - After all times on file2 (INTERPOLATING)",
			  time_step1,
			  file1.Time(time_step1));
		} else if (t2.step1 == t2.step2) {
		  sprintf(buf, "  --------- Time step %d, %13.7e ~ Matches step %d, %13.7e on file2 %s diff: %12.5e",
			  time_step1, file1.Time(time_step1),
			  t2.step1,   file2.Time(t2.step1),
			  interface.time_tol.abrstr(),
			  FileDiff(file1.Time(time_step1), file2.Time(t2.step1), interface.time_tol.type));
		} else {
		  sprintf(buf, "  --------- Time step %d, %13.7e ~ Interpolating step %d, %13.7e and step %d, %13.7e, proportion %10.4e on file2",
			  time_step1, file1.Time(time_step1),
			  t2.step1,   file2.Time(t2.step1),
			  t2.step2,  file2.Time(t2.step2),
			  t2.proportion);
		}
	      } else {
		sprintf(buf, "  --------- Time step %d, %13.7e ~ %13.7e, %s diff: %12.5e",
			time_step1,
			file1.Time(time_step1), file2.Time(time_step2),
			interface.time_tol.abrstr(),
			FileDiff(file1.Time(time_step1), file2.Time(time_step2), interface.time_tol.type));
	      }
	      std::cout << buf;
	    }

	    if (!interface.interpolating && interface.time_tol.Diff(file1.Time(time_step1), file2.Time(time_step2))) {
	      diff_flag = true;
	      if (interface.quiet_flag) Die_TS(time_step1);
	      else
		std::cout << " (FAILED) " << std::endl;
	    }
	    else if (!interface.quiet_flag)
	      std::cout <<   " ---------" << std::endl;

	    if (interface.interpolating && time_step == min_num_times) {
	      // last time.  Check if final database times match within specified tolerance...
	      int final2 = file2.Num_Times();
	      if (interface.final_time_tol.Diff(file1.Time(time_step1), file2.Time(final2))) {
		diff_flag = true;
		std::cout << "\tFinal database times differ by "
			  << FileDiff(file1.Time(time_step1), file2.Time(final2), interface.final_time_tol.type)
			  << " which is not within specified " << interface.final_time_tol.typestr() << " tolerance of "
			  << interface.final_time_tol.value
			  << " (FAILED)\n";
	      }
	    }
	  }

	  if (out_file_id >= 0) {
	    double t = file1.Time(time_step1);
	    ex_put_time(out_file_id, time_step1, &t);
	  }

	  if (interface.interpolating && (t2.step1 == -1 || t2.step2 == -1)) {
	    continue;
	  }

	  do_diffs(file1, file2, time_step1, t2, out_file_id,
		   mm_glob, mm_node, mm_elmt, mm_ns, mm_ss,
		   node_map, node_id_map, elmt_map, elem_id_map,
		   blocks2, var_vals, &diff_flag);

	}  // End of time step loop.

      // Make sure there is an operation to perform (compare times, variables, ...)
      if ((min_num_times == 0 && interface.coord_tol.type == IGNORE) ||
	  (min_num_times >  0 && interface.time_tol.type == IGNORE &&
	   interface.glob_var_names.size() == 0 &&
	   interface.node_var_names.size() == 0 &&
	   interface.elmt_var_names.size() == 0 &&
	   interface.elmt_att_names.size() == 0 &&
	   interface.ns_var_names.size()   == 0 &&
	   interface.ss_var_names.size()   == 0)) {
	std::cout << "\nWARNING: No comparisons were performed during "
	  "this execution.\n";
	diff_flag = true;
      }

    }

    if (interface.summary_flag) {
      output_summary( file1, mm_time, mm_glob, mm_node, mm_elmt, mm_ns,
		      mm_ss, node_id_map, elem_id_map );
    }
    else if (out_file_id >= 0) {
      ex_close(out_file_id);
    }
    else if (diff_flag) {
      std::cout << "\nexodiff: Files are different" << std::endl;
    }
    else if (file1.Num_Times() != file2.Num_Times()) {
      if ((file1.Num_Times() - interface.time_step_offset == file2.Num_Times()) ||
	  (interface.time_step_stop > 0) ||
	  (interface.explicit_steps.first != 0 && interface.explicit_steps.second != 0)) {
	std::cout << "\nexodiff: Files are the same" << std::endl;
	std::cout << "         The number of timesteps are different but "
		  << "the timesteps that were compared are the same."
		  << std::endl;
      } else {
	std::cout << "\nexodiff: Files are different (# time steps differ)"
		  << std::endl;
	diff_flag = true;
      }
    } else {
      std::cout << "\nexodiff: Files are the same" << std::endl;
    }

    if (!interface.ignore_maps) {
      file1.Free_Node_Map();
      file1.Free_Elmt_Map();
    } else {
      delete [] node_id_map;
      delete [] elem_id_map;
    }

    delete [] var_vals;
    delete [] blocks2;
    delete [] node_map;
    delete [] elmt_map;

    delete [] mm_glob;
    delete [] mm_node;
    delete [] mm_elmt;
    delete [] mm_ns;
    delete [] mm_ss;

    file1.Close_File();
    if (!interface.summary_flag)
      file2.Close_File();

    return diff_flag;
  }
}
  double FileDiff(double v1, double v2, TOLERANCE_TYPE_enum type)
  {
    if (type == IGNORE)          // ignore
      return 0.0;
    else if (type == RELATIVE) {      // relative diff
      if (v1 == 0.0 && v2 == 0.0) return 0.0;
      double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
      return (v1 - v2)/max;
    }
    else if (type == COMBINED) {
      // if (Abs(x - y) <= Max(absTol, relTol * Max(Abs(x), Abs(y))))
      // In the current implementation, absTol == relTol;
      // In summary, use abs tolerance if both values are less than 1.0;
      // else use relative tolerance.

      double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
      double tol = 1.0 < max ? max : 1.0;
      return fabs(v1 - v2) / tol;
    }
    else if (type == ABSOLUTE) {
      return (v1 - v2);
    }
    else if (type == EIGEN_REL) {      // relative diff
      if (v1 == 0.0 && v2 == 0.0) return 0.0;
      double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
      return (fabs(v1) - fabs(v2))/max;
    }
    else if (type == EIGEN_COM) {
      // if (Abs(x - y) <= Max(absTol, relTol * Max(Abs(x), Abs(y))))
      // In the current implementation, absTol == relTol;
      // In summary, use abs tolerance if both values are less than 1.0;
      // else use relative tolerance.

      double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
      double tol = 1.0 < max ? max : 1.0;
      return fabs(fabs(v1) - fabs(v2)) / tol;
    }
    else if (type == EIGEN_ABS) {
      return (fabs(v1) - fabs(v2));
    }
    else
      return 0.0;
  }

  void Die_TS(double ts)
  {
    std::cout << "exodiff: Files are different (time step " << ts << ")"
	      << std::endl;
    if (interface.exit_status_switch)
      exit(2);
    else
      exit(1);
  }

  template <typename INT>
  size_t global_elmt_num(ExoII_Read<INT>& file, size_t b_idx, size_t e_idx)
  {
    SMART_ASSERT(b_idx < file.Num_Elmt_Blocks());

    size_t g = 0;
    for (size_t b = 0; b < file.Num_Elmt_Blocks(); ++b)
      if (b_idx == b) {
	//std::cout << "returning " << g << " + " << e_idx << " + 1" << std::endl;
	return g + e_idx + 1;
      }
      else {
	SMART_ASSERT(file.Get_Elmt_Block_by_Index(b) != 0);
	//std::cout << "num elmts for block " << file.Get_Elmt_Block_by_Index(b)->Id()
	//     << " = " << file.Get_Elmt_Block_by_Index(b)->Size() << std::endl;
	g += file.Get_Elmt_Block_by_Index(b)->Size();
      }
    SMART_ASSERT(0);
    return 0;
  }

#if defined(__APPLE__) && !defined(isnan)
  /* for Mac OSX 10, this isnan function may need to be manually declared;
   * however, on some g++ versions, isnan is a macro so it doesn't need
   * to be manually declared...
   */
  extern "C" int isnan(double value);
#endif

  bool Invalid_Values(const double *values, size_t count)
  {
    bool valid = true;
    if (!interface.ignore_nans) {
      checking_invalid = true;
      invalid_data = false;

      SMART_ASSERT(values != NULL);


      for (size_t i=0; i < count; i++) {
#if (defined(__GNUC__) && (__GNUC__ == 2 && __GNUC_MINOR__ == 96)) || (defined(linux) && __PGI) || (defined(linux) && __INTEL_COMPILER)
	if (__isnan(values[i]))
#elif defined(interix)
	  if (values[i] != values[i])
#else
	    if (isnan(values[i]))
#endif
	      {
		valid = false;
		break;
	      }
	if (invalid_data) {  // may get set by SIGFPE handler
	  valid = false;
	  break;
	}
      }

      checking_invalid = false;
      invalid_data = false;
    }
    return !valid;
  }

  template <typename INT>
  const double *get_nodal_values(ExoII_Read<INT> &filen, int time_step, size_t idx,
				 int fno, const string &name, bool *diff_flag)
    {
      const double *vals = NULL;
      if (fno == 1 || !interface.summary_flag) {
	filen.Load_Nodal_Results(time_step, idx);
	vals = filen.Get_Nodal_Results(idx);

	if (vals != NULL) {
	  if (Invalid_Values(vals, filen.Num_Nodes())) {
	    std::cout << "\tERROR: NaN found for variable "
		      << name << " in file " << fno << "\n";
	    *diff_flag = true;
	  }
	}
      }
      return vals;
    }

  template <typename INT>
  const double *get_nodal_values(ExoII_Read<INT> &filen, const TimeInterp &t, size_t idx,
				 int fno, const string &name, bool *diff_flag)
    {
      const double *vals = NULL;
      if (fno == 1 || !interface.summary_flag) {
	vals = filen.Get_Nodal_Results(t.step1, t.step2, t.proportion, idx);

	if (vals != NULL) {
	  if (Invalid_Values(vals, filen.Num_Nodes())) {
	    std::cout << "\tERROR: NaN found for variable "
		      << name << " in file " << fno << "\n";
	    *diff_flag = true;
	  }
	}
      }
      return vals;
    }

template <typename INT>
void do_diffs(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int time_step1, TimeInterp t2, int out_file_id,
	      MinMaxData *mm_glob, MinMaxData *mm_node, MinMaxData *mm_elmt,
	      MinMaxData *mm_ns, MinMaxData *mm_ss,
	      INT *node_map, const INT *node_id_map, INT *elmt_map, const INT *elem_id_map,
	      Exo_Block<INT> **blocks2, double *var_vals, bool *diff_flag)
{
  if ( diff_globals( file1, file2, time_step1, t2, out_file_id,
		     mm_glob, var_vals ) )
    *diff_flag = true;

  // Nodal variables.
  if ( diff_nodals( file1, file2, time_step1, t2, out_file_id,
		    node_map, node_id_map, mm_node, var_vals ) )
    *diff_flag = true;

  // Element variables.
  if ( diff_element( file1, file2, time_step1, t2, out_file_id,
		     elmt_map, elem_id_map, blocks2, mm_elmt, var_vals ) )
    *diff_flag = true;

  if (interface.map_flag != PARTIAL) {
    // Nodeset variables.
    if ( diff_nodeset( file1, file2, time_step1, t2, out_file_id,
		       node_id_map, mm_ns, var_vals ) )
      *diff_flag = true;

    // Sideset variables.
    if ( diff_sideset( file1, file2, time_step1, t2, out_file_id,
		       elem_id_map, mm_ss, var_vals ) )
      *diff_flag = true;
  } else {
    if (interface.ns_var_names.size() > 0 || interface.ss_var_names.size() > 0) {
      std::cout << "WARNING: nodeset and sideset variables not (yet) "
	"compared for partial map\n";
    }
  }
}

  template <typename INT>
  bool diff_globals(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp t2,
		    int out_file_id, MinMaxData *mm_glob, double *gvals)
  {
    bool diff_flag = false;

    // Global variables.
    file1.Load_Global_Results(step1);
    const double* vals1 = file1.Get_Global_Results();
    const double* vals2 = 0;
    if (!interface.summary_flag) {
      file2.Load_Global_Results(t2.step1, t2.step2, t2.proportion);
      vals2 = file2.Get_Global_Results();
    }

    // ----------------------------------------------------------------------
    // Output file containing differences...
    if (out_file_id >= 0) {
      if (interface.glob_var_names.size() > 0) {
	SMART_ASSERT(gvals != 0);
	for (unsigned out_idx = 0; out_idx < interface.glob_var_names.size(); ++out_idx) {
	  const string& name = (interface.glob_var_names)[out_idx];
	  int idx1 = find_string(file1.Global_Var_Names(), name, interface.nocase_var_names);
	  int idx2 = find_string(file2.Global_Var_Names(), name, interface.nocase_var_names);
	  SMART_ASSERT(idx1 >= 0 && idx2 >= 0);
	  gvals[out_idx] = FileDiff(vals1[idx1], vals2[idx2], interface.output_type);
	}
	ex_put_glob_vars(out_file_id, t2.step1, interface.glob_var_names.size(), gvals);
      }
      return diff_flag;
    }

    // --------------------------------------------------------------------
    // Summary output
    if (interface.summary_flag) {
      for (unsigned out_idx = 0; out_idx < interface.glob_var_names.size(); ++out_idx) {
	const string& name = (interface.glob_var_names)[out_idx];
	int idx1 = find_string(file1.Global_Var_Names(), name, interface.nocase_var_names);
	SMART_ASSERT(idx1 >= 0);

	mm_glob[out_idx].spec_min_max(vals1[idx1], step1);
      }
      return diff_flag;
    }

    // -------------------------------------------------------------------
    // Determine if any diffs and output to terminal
    int name_length = max_string_length(file1.Global_Var_Names())+1;
    if (!interface.quiet_flag && interface.glob_var_names.size() > 0)
      std::cout << "Global variables:" << std::endl;

    for (unsigned out_idx = 0; out_idx < interface.glob_var_names.size(); ++out_idx) {
      const string& name = (interface.glob_var_names)[out_idx];
      int idx1 = find_string(file1.Global_Var_Names(), name, interface.nocase_var_names);
      int idx2 = find_string(file2.Global_Var_Names(), name, interface.nocase_var_names);
      SMART_ASSERT(idx1 >= 0 && idx2 >= 0);

      if (Invalid_Values(&vals1[idx1], 1)) {
	std::cout << "\tERROR: NaN found for variable " << name << " in file 1\n";
	diff_flag = true;
      }

      if (Invalid_Values(&vals2[idx2], 1)) {
	std::cout << "\tERROR: NaN found for variable " << name << " in file 2\n";
	diff_flag = true;
      }

      if (interface.glob_var[out_idx].Diff(vals1[idx1], vals2[idx2])) {
	diff_flag = true;

	if (!interface.quiet_flag) {
	  sprintf(buf, "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (FAILED)",
		  name_length,
		  name.c_str(), interface.glob_var[out_idx].abrstr(),
		  vals1[idx1], vals2[idx2],
		  interface.glob_var[out_idx].Delta(vals1[idx1], vals2[idx2]) );
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(step1);
      }
    }
    return diff_flag;
  }

  template <typename INT>
  bool diff_nodals(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp t2,
		   int out_file_id, INT *node_map, const INT *id_map,
		   MinMaxData *mm_node, double *nvals)
  {
    bool diff_flag = false;

    // ---------------------------------------------------------------------
    // Output file containing differences...
    if (out_file_id >= 0) {
      SMART_ASSERT(nvals != 0);
      int step2 = t2.step1;
      for (unsigned n_idx = 0; n_idx < interface.node_var_names.size(); ++n_idx) {
	const string& name = (interface.node_var_names)[n_idx];
	int idx1 = find_string(file1.Nodal_Var_Names(), name, interface.nocase_var_names);
	int idx2 = find_string(file2.Nodal_Var_Names(), name, interface.nocase_var_names);
	SMART_ASSERT(idx1 >= 0 && idx2 >= 0);

	const double* vals1 = get_nodal_values(file1, step1, idx1, 1,
					       name, &diff_flag);
	const double* vals2 = get_nodal_values(file2, step2, idx2, 2,
					       name, &diff_flag);

	size_t ncount = file1.Num_Nodes();
	for (size_t n = 0; n < ncount; ++n) {

	  // Should this node be processed...
	  if (node_map == 0 || node_map[n]>=0){
	    INT n2 = node_map != 0 ? node_map[n] : n;
	    nvals[n] = FileDiff(vals1[n], vals2[n2], interface.output_type);
	  } else {
	    nvals[n] = 0.;
	  }
	} // End of node iteration...
	ex_put_var(out_file_id, step2, EX_NODAL, n_idx+1, 0, file1.Num_Nodes(), nvals);
      }
      file1.Free_Nodal_Results();
      file2.Free_Nodal_Results();
      return diff_flag;
    }

    // -------------------------------------------------------------------
    // Summary output
    if (interface.summary_flag) {
      for (unsigned n_idx = 0; n_idx < interface.node_var_names.size(); ++n_idx) {
	const string& name = (interface.node_var_names)[n_idx];
	int idx1 = find_string(file1.Nodal_Var_Names(), name, interface.nocase_var_names);
	SMART_ASSERT(idx1 >= 0);
	const double* vals1 = get_nodal_values(file1, step1, idx1, 1,
					       name, &diff_flag);

	size_t ncount = file1.Num_Nodes();
	for (size_t n = 0; n < ncount; ++n) {
	  mm_node[n_idx].spec_min_max(vals1[n], step1, n);
	}
      }
      file1.Free_Nodal_Results();
      return diff_flag;
    }

    SMART_ASSERT(!interface.summary_flag && out_file_id <0);
    // ----------------------------------------------------------------------
    // Determine if any diffs and output to terminal
    if (!interface.quiet_flag && interface.node_var_names.size() > 0)
      std::cout << "Nodal variables:" << std::endl;
    int name_length = max_string_length(file1.Nodal_Var_Names())+1;

    for (unsigned n_idx = 0; n_idx < interface.node_var_names.size(); ++n_idx) {
      const string& name = (interface.node_var_names)[n_idx];
      int idx1 = find_string(file1.Nodal_Var_Names(), name, interface.nocase_var_names);
      int idx2 = find_string(file2.Nodal_Var_Names(), name, interface.nocase_var_names);
      SMART_ASSERT(idx1 >= 0 && idx2 >= 0);

      const double* vals1 = get_nodal_values(file1, step1, idx1, 1, name, &diff_flag);
      const double* vals2 = get_nodal_values(file2, t2,    idx2, 2, name, &diff_flag);

      DiffData max_diff;
      double norm_d = 0.0;
      double norm_1 = 0.0;
      double norm_2 = 0.0;
      size_t ncount = file1.Num_Nodes();
      for (size_t n = 0; n < ncount; ++n) {

	// Should this node be processed...
	if (node_map == 0 || node_map[n]>=0){
	  INT n2 = node_map != 0 ? node_map[n] : n;
	  double d = interface.node_var[n_idx].Delta(vals1[n], vals2[n2]);
	  if (interface.show_all_diffs) {
	    if (d > interface.node_var[n_idx].value) {
	      diff_flag = true;
	      sprintf(buf, "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (node %lu)",
		      name_length,
		      name.c_str(), interface.node_var[n_idx].abrstr(),
		      vals1[n], vals2[n2], d, (size_t)id_map[n]);
	      std::cout << buf << std::endl;
	    }
	  } else {
	    max_diff.set_max(d, vals1[n], vals2[n2], n);
	  }
	  if (interface.doNorms) {
	    norm_d += (vals1[n]-vals2[n2])*(vals1[n]-vals2[n2]);
	    norm_1 += vals1[n]*vals1[n];
	    norm_2 += vals2[n2]*vals2[n2];
	  }
	}
      } // End of node iteration...

      if (interface.doNorms && norm_d > 0.0) {
	norm_d = sqrt(norm_d);
	norm_1 = sqrt(norm_1);
	norm_2 = sqrt(norm_2);
        sprintf(buf,
                "   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e) rel=%14.7e",
                name_length, name.c_str(),
		norm_d, norm_1, norm_2, norm_d / max(norm_1, norm_2));
        std::cout << buf << std::endl;
      }

      if (max_diff.diff > interface.node_var[n_idx].value) {
	diff_flag = true;
	if (!interface.quiet_flag) {
	  sprintf(buf, "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (node %lu)",
		  name_length,
		  name.c_str(), interface.node_var[n_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, (size_t)id_map[max_diff.id]);
	  std::cout << buf << std::endl;
	}
	else {
	  Die_TS(step1);
	}
      }
    }
    file1.Free_Nodal_Results();
    file2.Free_Nodal_Results();
    return diff_flag;
  }

  template <typename INT>
  bool diff_element(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp t2,
		    int out_file_id, INT* elmt_map, const INT *id_map,
		    Exo_Block<INT> **blocks2, MinMaxData *mm_elmt, double *evals)
  {
    bool diff_flag = false;

    if (out_file_id >= 0) {SMART_ASSERT(evals != 0);}

    if (out_file_id < 0 && !interface.quiet_flag && !interface.summary_flag && interface.elmt_var_names.size() > 0)
      std::cout << "Element variables:" << std::endl;

    int name_length = max_string_length(interface.elmt_var_names)+1;

    for (unsigned e_idx = 0; e_idx < interface.elmt_var_names.size(); ++e_idx) {
      const string& name = (interface.elmt_var_names)[e_idx];
      int vidx1 = find_string(file1.Elmt_Var_Names(), name, interface.nocase_var_names);
      int vidx2 = 0;
      if (!interface.summary_flag)
	vidx2 = find_string(file2.Elmt_Var_Names(), name, interface.nocase_var_names);
      SMART_ASSERT(vidx1 >= 0 && vidx2 >= 0);

      double norm_d = 0.0;
      double norm_1 = 0.0;
      double norm_2 = 0.0;

      if (elmt_map != 0) { // Load variable for all blocks in file 2.
	for (int b = 0; b < file2.Num_Elmt_Blocks(); ++b) {
	  Exo_Block<INT> * block2 = file2.Get_Elmt_Block_by_Index(b);
	  block2->Load_Results(t2.step1, t2.step2, t2.proportion, vidx2);
	}
      }

      size_t global_elmt_index = 0;
      DiffData max_diff;
      size_t e2;
      for (int b = 0; b < file1.Num_Elmt_Blocks(); ++b) {
	Exo_Block<INT>* eblock1 = file1.Get_Elmt_Block_by_Index(b);
	if (!eblock1->is_valid_var(vidx1)) {
	  global_elmt_index += eblock1->Size();
	  continue;
	}

	Exo_Block<INT>* eblock2 = NULL;
	int b2 = b;
	if (elmt_map == 0 && !interface.summary_flag) {
	  size_t id = eblock1->Id();
	  eblock2 = file2.Get_Elmt_Block_by_Id(id);
	  SMART_ASSERT(eblock2 != NULL);
	  if (!eblock2->is_valid_var(vidx2)) {
	    continue;
	  }
	}

	eblock1->Load_Results(step1, vidx1);
	const double* vals1 = eblock1->Get_Results(vidx1);

	if (vals1 != NULL) {
	  if (Invalid_Values(vals1, eblock1->Size())) {
	    std::cout << "\tERROR: NaN found for variable "
		      << name << " in block "
		      << eblock1->Id() << ", file 1\n";
	    diff_flag = true;
	  }
	}

	double v2 = 0;
	const double* vals2 = NULL;

	if (elmt_map == 0 && !interface.summary_flag) {
	  // Without mapping, get result for this block.
	  size_t id = eblock1->Id();
	  eblock2 = file2.Get_Elmt_Block_by_Id(id);
	  eblock2->Load_Results(t2.step1, t2.step2, t2.proportion, vidx2);
	  vals2 = eblock2->Get_Results(vidx2);

	  if (vals2 != NULL) {
	    if (Invalid_Values(vals2, eblock2->Size())) {
	      std::cout << "\tERROR: NaN found for variable "
			<< name << " in block "
			<< eblock2->Id() << ", file 2\n";
	      diff_flag = true;
	    }
	  }
	}

	size_t ecount =  eblock1->Size();
	size_t block_id = eblock1->Id();
	for (size_t e = 0; e < ecount; ++e) {
	  if (out_file_id >= 0)evals[e] = 0.;
	  INT el_flag = 1;
	  if(elmt_map != 0)
	    el_flag = elmt_map[global_elmt_index];

	  if(el_flag >= 0){
	    if (!interface.summary_flag) {
	      if (elmt_map == 0)
		v2 = vals2[e];
	      else {
		// With mapping, map global index from file 1 to global index
		// for file 2.  Then convert to block index and elmt index.
		file2.Global_to_Block_Local(
					    elmt_map[global_elmt_index] + 1, b2, e2);
		SMART_ASSERT(blocks2[b2] != 0);
		SMART_ASSERT(blocks2[b2]->Get_Results(vidx2) != 0);
		v2 = blocks2[b2]->Get_Results(vidx2)[e2]; // Get value from file 2.
	      }
	    }

	    if (interface.summary_flag) {
	      mm_elmt[e_idx].spec_min_max(vals1[e], step1,
					  global_elmt_index, block_id);
	    }
	    else if (out_file_id >= 0) {
	      evals[e] = FileDiff(vals1[e], v2, interface.output_type);
	    }
	    else if (interface.show_all_diffs) {
	      double d = interface.elmt_var[e_idx].Delta(vals1[e], v2);
	      if (d > interface.elmt_var[e_idx].value) {
		diff_flag = true;
		sprintf(buf,
			"   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (block %lu, elmt %lu)",
			name_length, name.c_str(),
			interface.elmt_var[e_idx].abrstr(),
			vals1[e], v2, d, block_id, (size_t)id_map[global_elmt_index]);
		std::cout << buf << std::endl;
	      }
	    }
	    else {
	      double d = interface.elmt_var[e_idx].Delta(vals1[e], v2);
	      max_diff.set_max(d, vals1[e], v2, global_elmt_index, block_id);
	    }
	    if (interface.doNorms) {
	      norm_d += (vals1[e]-v2)*(vals1[e]-v2);
	      norm_1 += vals1[e]*vals1[e];
	      norm_2 += v2*v2;
	    }
	  }
	  ++global_elmt_index;
	}

	if (out_file_id >= 0)
	  ex_put_var(out_file_id, t2.step1, EX_ELEM_BLOCK, e_idx+1,
		     eblock1->Id(), eblock1->Size(), evals);

	eblock1->Free_Results();
	if (!interface.summary_flag && elmt_map == 0) {
	  eblock2->Free_Results();
	}

      }  // End of element block loop.

      if (interface.doNorms && norm_d > 0.0) {
	norm_d = sqrt(norm_d);
	norm_1 = sqrt(norm_1);
	norm_2 = sqrt(norm_2);
	sprintf(buf,
                "   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e) rel=%14.7e",
                name_length, name.c_str(),
		norm_d, norm_1, norm_2, norm_d / max(norm_1, norm_2));
        std::cout << buf << std::endl;
      }

      if (!interface.summary_flag && max_diff.diff > interface.elmt_var[e_idx].value) {
	diff_flag = true;

	if (!interface.quiet_flag) {
	  sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (block %lu, elmt %lu)",
		  name_length, name.c_str(),
		  interface.elmt_var[e_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, max_diff.blk, (size_t)id_map[max_diff.id]);
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(step1);
      }

    }  // End of element variable loop.
    return diff_flag;
  }

  template <typename INT>
  bool diff_nodeset(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp t2,
		    int out_file_id, const INT *id_map,
		    MinMaxData *mm_ns, double *vals)
  {
    string serr;
    bool diff_flag = false;

    if (out_file_id >= 0) {SMART_ASSERT(vals != 0);}

    int name_length = max_string_length(file1.NS_Var_Names())+1;

    if ( out_file_id < 0 && !interface.quiet_flag &&
	 !interface.summary_flag && interface.ns_var_names.size() > 0 )
      std::cout << "Nodeset variables:" << std::endl;

    for (unsigned e_idx = 0; e_idx < interface.ns_var_names.size(); ++e_idx) {
      const string& name = (interface.ns_var_names)[e_idx];
      int vidx1 = find_string(file1.NS_Var_Names(), name, interface.nocase_var_names);
      int vidx2 = 0;
      if (!interface.summary_flag)
	vidx2 = find_string(file2.NS_Var_Names(), name, interface.nocase_var_names);
      SMART_ASSERT(vidx1 >= 0 && vidx2 >= 0);

      double norm_d = 0.0;
      double norm_1 = 0.0;
      double norm_2 = 0.0;

      DiffData max_diff;
      for (int b = 0; b < file1.Num_Node_Sets(); ++b) {
	Node_Set<INT>* nset1 = file1.Get_Node_Set_by_Index(b);
	if (!nset1->is_valid_var(vidx1)) {
	  continue;
	}

	Node_Set<INT>* nset2 = NULL;
	if (!interface.summary_flag) {
	  size_t id = nset1->Id();
	  nset2 = file2.Get_Node_Set_by_Id(id);
	  SMART_ASSERT(nset2 != NULL);
	  SMART_ASSERT(nset2->Id() == nset1->Id());
	  if (!nset2->is_valid_var(vidx2)) continue;
	}

	nset1->Load_Results(step1, vidx1);
	const double* vals1 = nset1->Get_Results(vidx1);

	if (vals1 != NULL) {
	  if (Invalid_Values(vals1, nset1->Size())) {
	    std::cout << "\tERROR: NaN found for variable "
		      << name << " in nodeset "
		      << nset1->Id() << ", file 1\n";
	    diff_flag = true;
	  }
	}

	double v2 = 0;
	double* vals2 = 0;
	if (!interface.summary_flag) {
	  // Without mapping, get result for this nset
	  nset2->Load_Results(t2.step1, t2.step2, t2.proportion, vidx2);
	  vals2 = (double*)nset2->Get_Results(vidx2);

	  if (vals2 != NULL) {
	    if (Invalid_Values(vals2, nset2->Size())) {
	      std::cout << "\tERROR: NaN found for variable "
			<< name << " in nodeset "
			<< nset2->Id() << ", file 2\n";
	      diff_flag = true;
	    }
	  }
	}

	size_t ncount = nset1->Size();
	for (size_t e = 0; e < ncount; ++e) {
	  int idx1 = nset1->Node_Index(e);
	  int idx2 = 0;

	  if (out_file_id >= 0) vals[idx1] = 0.;
	  if (!interface.summary_flag) {
	    idx2 = nset2->Node_Index(e);
	    v2 = vals2[idx2];
	  }

	  if (interface.summary_flag) {
	    mm_ns[e_idx].spec_min_max(vals1[idx1], step1, e, nset1->Id());
	  }
	  else if (out_file_id >= 0) {
	    vals[idx1] = FileDiff(vals1[idx1], v2, interface.output_type);
	  }
	  else if (interface.show_all_diffs) {
	    double d = interface.ns_var[e_idx].Delta(vals1[idx1], v2);
	    if (d > interface.ns_var[e_idx].value) {
	      diff_flag = true;
	      sprintf(buf,
		      "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (set %lu, node %lu)",
		  name_length, name.c_str(), interface.ns_var[e_idx].abrstr(),
		      vals1[idx1], v2, d, nset1->Id(), e);
	      std::cout << buf << std::endl;
	    }
	  }
	  else {
	    double d = interface.ns_var[e_idx].Delta(vals1[idx1], v2);
	    max_diff.set_max(d, vals1[idx1], v2, e, nset1->Id());
	  }
	  if (interface.doNorms) {
	    norm_d += (vals1[idx1]-v2)*(vals1[idx1]-v2);
	    norm_1 += vals1[idx1]*vals1[idx1];
	    norm_2 += v2*v2;
	  }
	}

	if (out_file_id >= 0)
	  ex_put_var(out_file_id, t2.step1, EX_NODE_SET,
		     e_idx+1, nset1->Id(),
		     nset1->Size(), vals);

	nset1->Free_Results();
	if (!interface.summary_flag) {
	  nset2->Free_Results();
	}

      }  // End of nodeset loop.

      if (interface.doNorms && norm_d > 0.0) {
        sprintf(buf,
                "   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e)",
                name_length, name.c_str(),
		sqrt(norm_d), sqrt(norm_1), sqrt(norm_2));
        std::cout << buf << std::endl;
      }

      if (!interface.summary_flag && max_diff.diff > interface.ns_var[e_idx].value) {
	diff_flag = true;

	if (!interface.quiet_flag) {
	  Node_Set<INT> *nset = file1.Get_Node_Set_by_Id(max_diff.blk);
	  sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (set %lu, node %lu)",
		  name_length,
		  name.c_str(),
		  interface.ns_var[e_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, max_diff.blk,
		  (size_t)id_map[nset->Node_Id(max_diff.id)-1]);
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(step1);
      }
    }  // End of nodeset variable loop.
    return diff_flag;
  }

  template <typename INT>
  bool diff_sideset(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2, int step1, TimeInterp t2,
		    int out_file_id, const INT *id_map,
		    MinMaxData *mm_ss, double *vals)
  {
    string serr;
    bool diff_flag = false;

    if (out_file_id >= 0) {SMART_ASSERT(vals != 0);}

    int name_length = max_string_length(file1.SS_Var_Names())+1;

    if (out_file_id < 0 && !interface.quiet_flag && !interface.summary_flag && interface.ss_var_names.size() > 0)
      std::cout << "Sideset variables:" << std::endl;

    double norm_d = 0.0;
    double norm_1 = 0.0;
    double norm_2 = 0.0;

    for (unsigned e_idx = 0; e_idx < interface.ss_var_names.size(); ++e_idx) {
      const string& name = (interface.ss_var_names)[e_idx];
      int vidx1 = find_string(file1.SS_Var_Names(), name, interface.nocase_var_names);
      int vidx2 = 0;
      if (!interface.summary_flag)
	vidx2 = find_string(file2.SS_Var_Names(), name, interface.nocase_var_names);
      SMART_ASSERT(vidx1 >= 0 && vidx2 >= 0);

      DiffData max_diff;
      for (int b = 0; b < file1.Num_Side_Sets(); ++b) {
	Side_Set<INT>* sset1 = file1.Get_Side_Set_by_Index(b);
	SMART_ASSERT(sset1 != NULL);
	if (!sset1->is_valid_var(vidx1)) {
	  continue;
	}

	Side_Set<INT>* sset2 = NULL;
	if (!interface.summary_flag) {
	  size_t id = sset1->Id();
	  sset2 = file2.Get_Side_Set_by_Id(id);
	  if (sset2 == NULL || !sset2->is_valid_var(vidx2)) continue;
	}

	sset1->Load_Results(step1, vidx1);
	const double* vals1 = sset1->Get_Results(vidx1);

	if (vals1 != NULL) {
	  if (Invalid_Values(vals1, sset1->Size())) {
	    std::cout << "\tERROR: NaN found for variable "
		      << name << " in sideset "
		      << sset1->Id() << ", file 1\n";
	    diff_flag = true;
	  }
	}

	double v2 = 0;
	double* vals2 = 0;
	if (!interface.summary_flag) {
	  sset2->Load_Results(t2.step1, t2.step2, t2.proportion, vidx2);
	  vals2 = (double*)sset2->Get_Results(vidx2);

	  if (vals2 != NULL) {
	    if (Invalid_Values(vals2, sset2->Size())) {
	      std::cout << "\tERROR: NaN found for variable "
			<< name << " in sideset "
			<< sset2->Id() << ", file 2\n";
	      diff_flag = true;
	    }
	  }
	}

	size_t ecount = sset1->Size();
	for (size_t e = 0; e < ecount; ++e) {
	  size_t ind1 = sset1->Side_Index(e);
	  size_t ind2 = 0;
	  if (out_file_id >= 0) vals[ind1] = 0.;
	  if (!interface.summary_flag) {
	    ind2 = sset2->Side_Index(e);
	    v2 = vals2[ind2];
	  }

	  if (interface.summary_flag) {
	    mm_ss[e_idx].spec_min_max(vals1[ind1], step1, e, sset1->Id());
	  }
	  else if (out_file_id >= 0) {
	    vals[ind1] = FileDiff(vals1[ind1], v2, interface.output_type);
	  }
	  else if (interface.show_all_diffs) {
	    double d = interface.ss_var[e_idx].Delta(vals1[ind1], v2);
	    if (d > interface.ss_var[e_idx].value) {
	      diff_flag = true;
	      sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (set %lu, side %lu.%d)",
		      name_length, name.c_str(), interface.ss_var[e_idx].abrstr(),
		      vals1[ind1], v2, d,
		      (size_t)sset1->Id(),
		      (size_t)id_map[sset1->Side_Id(e).first-1],
		      (int)sset1->Side_Id(e).second);
	      std::cout << buf << std::endl;
	    }
	  }
	  else {
	    double d = interface.ss_var[e_idx].Delta(vals1[ind1], v2);
	    max_diff.set_max(d, vals1[ind1], v2, e, sset1->Id());
	  }
	  if (interface.doNorms) {
	    norm_d += (vals1[ind1]-v2)*(vals1[ind1]-v2);
	    norm_1 += vals1[ind1]*vals1[ind1];
	    norm_2 += v2*v2;
	  }
	}

	if (out_file_id >= 0)
	  ex_put_var(out_file_id, t2.step1, EX_SIDE_SET,
		     e_idx+1, sset1->Id(),
		     sset1->Size(), vals);
	sset1->Free_Results();
	if (!interface.summary_flag) {
	  sset2->Free_Results();
	}
      }  // End of sideset loop.

      if (!interface.summary_flag && max_diff.diff > interface.ss_var[e_idx].value) {
	diff_flag = true;

	if (interface.doNorms && norm_d > 0.0) {
	  sprintf(buf,
		  "   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e)",
		  name_length, name.c_str(),
		  sqrt(norm_d), sqrt(norm_1), sqrt(norm_2));
	  std::cout << buf << std::endl;
	}

	if (!interface.quiet_flag) {
	  Side_Set<INT> *sset = file1.Get_Side_Set_by_Id(max_diff.blk);
	  sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (set %lu, side %lu.%d)",
		  name_length,
		  name.c_str(),
		  interface.ss_var[e_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, max_diff.blk,
		  (size_t)id_map[sset->Side_Id(max_diff.id).first-1],
		  (int)sset->Side_Id(max_diff.id).second);
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(step1);
      }

    }  // End of sideset variable loop.
    return diff_flag;
  }

template <typename INT>
bool diff_element_attributes(ExoII_Read<INT>& file1, ExoII_Read<INT>& file2,
                             INT* /*elmt_map*/, const INT *id_map,
                             Exo_Block<INT> ** /*blocks2*/)
{
  bool diff_was_output = false;
  bool diff_flag = false;

  size_t global_elmt_offset = 0;
  for (int b = 0; b < file1.Num_Elmt_Blocks(); ++b) {
    Exo_Block<INT>* eblock1 = file1.Get_Elmt_Block_by_Index(b);
    SMART_ASSERT(eblock1 != NULL);

    size_t block_id = eblock1->Id();

    Exo_Block<INT> *eblock2 = file2.Get_Elmt_Block_by_Id(block_id);
    SMART_ASSERT(eblock2 != NULL);

    if (!diff_was_output && (eblock1->attr_count() > 0 || eblock2->attr_count() > 0)) {
      diff_was_output = true;
      std::cout << "Element attributes:" << std::endl;
    }

    int name_length = max_string_length(eblock1->Attribute_Names())+1;
    for (int idx1=0; idx1 < eblock1->attr_count(); idx1++) {
      size_t global_elmt_index = global_elmt_offset;

      DiffData max_diff;
      const std::string &name = eblock1->Get_Attribute_Name(idx1);

      // Find same attribute in eblock2...
      int idx2 = eblock2->Find_Attribute_Index(name);
      if (idx2 < 0) {
	continue;
      }

      // Find name in interface.elmt_att_names
      int tol_idx = -1;
      for (unsigned e_idx = 0; e_idx < interface.elmt_att_names.size(); ++e_idx) {
	if (name == (interface.elmt_att_names)[e_idx]) {
	  tol_idx = e_idx;
	  break;
	}
      }

      if (tol_idx == -1)
	continue;

      double norm_d = 0.0;
      double norm_1 = 0.0;
      double norm_2 = 0.0;

      eblock1->Load_Attributes(idx1);
      const double* vals1 = eblock1->Get_Attributes(idx1);

      if (vals1 != NULL) {
	if (Invalid_Values(vals1, eblock1->Size())) {
	  std::cout << "\tERROR: NaN found for attribute "
		    << name << " in block "
		    << eblock1->Id() << ", file 1\n";
	  diff_flag = true;
	}
      }

      // Without mapping, get result for this block.
      eblock2->Load_Attributes(idx2);
      const double* vals2 = eblock2->Get_Attributes(idx2);

      if (vals2 != NULL) {
	if (Invalid_Values(vals2, eblock2->Size())) {
	  std::cout << "\tERROR: NaN found for attribute "
		    << name << " in block "
		    << eblock2->Id() << ", file 2\n";
	  diff_flag = true;
	}
      }

      size_t ecount =  eblock1->Size();
      for (size_t e = 0; e < ecount; ++e) {

	if (interface.show_all_diffs) {
	  double d = interface.elmt_att[tol_idx].Delta(vals1[e], vals2[e]);
	  if (d > interface.elmt_att[tol_idx].value) {
	    diff_flag = true;
	    sprintf(buf,
		    "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (block %lu, elmt %lu)",
		    name_length, name.c_str(),
		    interface.elmt_att[tol_idx].abrstr(),
		    vals1[e], vals2[e], d, block_id, (size_t)id_map[global_elmt_index]);
	    std::cout << buf << std::endl;
	  }
	}
	else {
	  double d = interface.elmt_att[tol_idx].Delta(vals1[e], vals2[e]);
	  max_diff.set_max(d, vals1[e], vals2[e], global_elmt_index, block_id);
	}
	if (interface.doNorms) {
	  norm_d += (vals1[e]-vals2[e])*(vals1[e]-vals2[e]);
	  norm_1 += vals1[e]*vals1[e];
	  norm_2 += vals2[e]*vals2[e];
	}
	++global_elmt_index;
      }

      if (interface.doNorms && norm_d > 0.0) {
	norm_d = sqrt(norm_d);
	norm_1 = sqrt(norm_1);
	norm_2 = sqrt(norm_2);
	sprintf(buf,
		"   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e) rel=%14.7e",
		name_length, name.c_str(),
		norm_d, norm_1, norm_2, norm_d / max(norm_1, norm_2));
	std::cout << buf << std::endl;
      }

      if (!interface.summary_flag && max_diff.diff > interface.elmt_att[tol_idx].value) {
	diff_flag = true;

	if (!interface.quiet_flag) {
	  sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (block %lu, elmt %lu)",
		  name_length, name.c_str(),
		  interface.elmt_att[tol_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, max_diff.blk, (size_t)id_map[max_diff.id]);
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(-1);
      }
    }  // End of attribute loop.
    eblock1->Free_Attributes();
    eblock2->Free_Attributes();

    global_elmt_offset += eblock1->Size();
  }  // End of element block loop.
  return diff_flag;
}

  template <typename INT>
  void output_summary(ExoII_Read<INT>& file1, MinMaxData &mm_time, MinMaxData *mm_glob,
		      MinMaxData *mm_node, MinMaxData *mm_elmt,
		      MinMaxData *mm_ns, MinMaxData *mm_ss,
		      const INT *node_id_map, const INT *elem_id_map)
  {
    int name_length = 0;
    int i, n;

    std::cout << "# NOTES:  - The min/max values are reporting the min/max "
	      << "in absolute value.\n"
	      << "#         - Time values (t) are 1-offset time step numbers.\n"
	      << "#         - Element block numbers are the block ids.\n"
	      << "#         - Node(n) and element(e) numbers are 1-offset."
	      << std::endl;

    if (interface.coord_sep) {
      double min_separation = Find_Min_Coord_Sep(file1);
      std::cout << "\nCOORDINATES absolute 1.e-6    # min separation = "
		<< min_separation << "\n";
    } else {
      std::cout << "\nCOORDINATES absolute 1.e-6    # min separation "
	"not calculated\n";
    }

    if (file1.Num_Times() > 0) {
      std::cout << "\nTIME STEPS relative 1.e-6 floor 0.0     # min: ";
      sprintf(buf, "%15.8g @ t%d max: %15.8g @ t%d\n",
	      mm_time.min_val, mm_time.min_step, mm_time.max_val,
	      mm_time.max_step);
      std::cout << buf << std::endl;
    } else {
      std::cout << "\n# No TIME STEPS\n";
    }

    n = interface.glob_var_names.size();
    if (n > 0) {
      std::cout << "GLOBAL VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(interface.glob_var_names);
      for (i = 0; i < n; ++i) {
	sprintf(buf, "\t%-*s  # min: %15.8g @ t%d\tmax: %15.8g @ t%d\n",
		name_length, ((interface.glob_var_names)[i]).c_str(),
		mm_glob[i].min_val, mm_glob[i].min_step,
		mm_glob[i].max_val, mm_glob[i].max_step);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No GLOBAL VARIABLES\n";
    }

    n = interface.node_var_names.size();
    if (n > 0) {
      std::cout << std::endl << "NODAL VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(interface.node_var_names);
      for (i = 0; i < n; ++i) {
	sprintf(buf, "\t%-*s  # min: %15.8g @ t%d,n%lu\tmax: %15.8g @ t%d,n%lu\n",
		name_length, ((interface.node_var_names)[i]).c_str(),
		mm_node[i].min_val, mm_node[i].min_step,
		(size_t)node_id_map[mm_node[i].min_id],
		mm_node[i].max_val, mm_node[i].max_step,
		(size_t)node_id_map[mm_node[i].max_id]);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No NODAL VARIABLES\n";
    }

    n = interface.elmt_var_names.size();
    if (n > 0) {
      std::cout << std::endl << "ELEMENT VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(interface.elmt_var_names);
      for (i = 0; i < n; ++i) {
	sprintf(buf, "\t%-*s  # min: %15.8g @ t%d,b%lu,e%lu\tmax: %15.8g @ t%d,b%lu,e%lu\n",
		name_length, ((interface.elmt_var_names)[i]).c_str(),
		mm_elmt[i].min_val, mm_elmt[i].min_step,
		mm_elmt[i].min_blk, (size_t)elem_id_map[mm_elmt[i].min_id],
		mm_elmt[i].max_val, mm_elmt[i].max_step,
		mm_elmt[i].max_blk, (size_t)elem_id_map[mm_elmt[i].max_id]);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No ELEMENT VARIABLES\n";
    }

    n = interface.ns_var_names.size();
    if (n > 0) {
      std::cout << std::endl << "NODESET VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(interface.ns_var_names);
      for (i = 0; i < n; ++i) {
	Node_Set<INT> *nsmin = file1.Get_Node_Set_by_Id(mm_ns[i].min_blk);
	Node_Set<INT> *nsmax = file1.Get_Node_Set_by_Id(mm_ns[i].max_blk);
	sprintf(buf,
		"\t%-*s  # min: %15.8g @ t%d,s%lu,n%lu\tmax: %15.8g @ t%d,s%lu,n%lu\n",
		name_length, ((interface.ns_var_names)[i]).c_str(),
		mm_ns[i].min_val, mm_ns[i].min_step, mm_ns[i].min_blk,
		(size_t)node_id_map[nsmin->Node_Id(mm_ns[i].min_id)-1],
		mm_ns[i].max_val, mm_ns[i].max_step, mm_ns[i].max_blk,
		(size_t)node_id_map[nsmax->Node_Id(mm_ns[i].max_id)-1]);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No NODESET VARIABLES\n";
    }

    n = interface.ss_var_names.size();
    if (n > 0) {
      std::cout << std::endl << "SIDESET VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(interface.ss_var_names);
      for (i = 0; i < n; ++i) {
	Side_Set<INT> *ssmin = file1.Get_Side_Set_by_Id(mm_ss[i].min_blk);
	Side_Set<INT> *ssmax = file1.Get_Side_Set_by_Id(mm_ss[i].max_blk);
	std::pair<int,int> min_side = ssmin->Side_Id(mm_ss[i].min_id);
	std::pair<int,int> max_side = ssmax->Side_Id(mm_ss[i].max_id);
	sprintf(buf,
		"\t%-*s  # min: %15.8g @ t%d,s%lu,f%lu.%d\tmax: %15.8g @ t%d,s%lu,f%lu.%d\n",
		name_length, ((interface.ss_var_names)[i]).c_str(),
		mm_ss[i].min_val, mm_ss[i].min_step, mm_ss[i].min_blk,
		(size_t)elem_id_map[min_side.first-1], min_side.second,
		mm_ss[i].max_val, mm_ss[i].max_step, mm_ss[i].max_blk,
		(size_t)elem_id_map[max_side.first-1], max_side.second);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No SIDESET VARIABLES\n";
    }
    std::cout << std::endl;
  }

  int timeStepIsExcluded(int ts)
  {
    for (size_t i = 0; i < interface.exclude_steps.size(); ++i)
      if (ts == interface.exclude_steps[i])
	return 1;

    return 0;
  }
