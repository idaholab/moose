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
#include "exodusII.h"
#include "stringx.h"
#include "Specifications.h"
#include "parsing.h"
#include "FileInfo.h"

 
using namespace std;

string Version() { return "2.46 (2011-07-11)"; }

string Date() {
  char tbuf[32];
  time_t calendar_time = time(NULL);
  struct tm *local_time = localtime(&calendar_time);
  strftime(tbuf, 32, "%Y/%m/%d   %H:%M:%S %Z", local_time);
  string time_string(tbuf);
  return time_string;
}

bool Invalid_Values(const double *values, int count);

void Print_Banner(const char *prefix)
{
  std::cout
  << "\n"
  << prefix << "  *****************************************************************\n"
  << prefix <<  "    EXODIFF  EXODIFF  EXODIFF  EXODIFF  EXODIFF  EXODIFF  EXODIFF  \n"
  << prefix <<  "                                                                   \n"
  << prefix <<  "                       Version " << Version()                 << "\n"
  << prefix <<  "             Authors:  Richard Drake, rrdrake@sandia.gov           \n"
  << prefix <<  "                       Greg Sjaardema, gdsjaar@sandia.gov          \n"
  << prefix <<  "                       " << Date()                           << "\n"
  << prefix <<  "                                                                   \n"
  << prefix <<  "    EXODIFF  EXODIFF  EXODIFF  EXODIFF  EXODIFF  EXODIFF  EXODIFF  \n"
  << prefix <<  "  *****************************************************************\n"
  << std::endl;
}

void Echo_Usage() { std::cout
<< "Usage:  exodiff [-t <real value>] [-T <offset>|-TA|-TM] [-q] [-relative] [-f <cmd file>] [-m] [-s] [-i] [-norms] [-stat]\n"
<< "                file1.exo file2.exo [diffile.exo]\n"
<< "   or:  exodiff -summary [no_coord_sep] <file.exo> (create variable summary)  \n"
<< "   or:  exodiff [-h] [-help]             (usage)                    \n"
<< "   or:  exodiff [-H]                     (man page)                 \n"
<< "   or:  exodiff [-v] [-version]                 " << std::endl;
}

void Echo_Help(const std::string &option) {
  Print_Banner(" ");
  Echo_Usage();
  if (option.empty() || option == "all") {
    std::cout
      << "\n"
      << " DESCRIPTION:\n\n"
      << "   Exodiff compares the results data from two Exodus II files.\n"
      << "   By default, a relative difference is used with a tolerance of 1.e-6\n"
      << "   (about 6 significant digits).  The nodal locations are also compared\n"
      << "   using an absolute difference with a tolerance of 1.e-6.  Time step\n"
      << "   values are compared using a relative difference with a tolerance of\n"
      << "   1.e-6 and a floor of 1.e-15.\n"
      << "\n"
      << "   If a third file [diffile.exo] is given, then it is overwritten with\n"
      << "   the difference of the two files.\n"
      << "\n"
      << "   A command file can be specified and used to control exactly what\n"
      << "   variables are to be compared/differenced and to what tolerance.\n"
      << "\n"
      << "   Relative differences are computed by |val1 - val2|/max(|val1|, |val2|).\n"
      << "   Absolute differences are computed by |val1 - val2| .\n"
      << "   Combined differences are computed by |val1 - val2| / max(tol, tol * max(|val1|, |val2|)\n"
      << "   Values are considered equal if |val1| <= floor && |val2| <= floor.\n"
      << "\n"
      << "   By default, element block names are compared fully but ignoring case.\n"
      << "   Variable names are compared fully but ignoring case.\n"
      << "\n"
      << " OPTIONS:\n\n"
      << "    -t <real value> : Overrides the default tolerance of 1.0E-6.\n"
      << "    -F <real value> : Overrides the default floor tolerance of 0.0.\n"
      << "\n"
      << "    -T <offset> : Timestep 'x+offset' in first file matches timestep 'x' in second file.\n"
      << "    -TA : Automatic determination of timestep offset -- end at same step.\n"
      << "    -TM : Automatic determination of timestep offset -- closest match to first step on file2.\n"
      << "\n"
      << "    -q : Quiet.  Only errors will be sent to stdout.  Comparison mode\n"
      << "         will echo \"exodiff: Files are the same.\" or \"exodiff: Files\n"
      << "         are different.\"\n"
      << "\n"
      << "    -absolute : Use absolute differences as default tolerance type.\n"
      << "    -relative : Use relative differences as default tolerance type.\n"
      << "    -combined : Use combined differences as default tolerance type.\n"
      << "    -eigen_absolute : Use eigen_absolute differences as default tolerance type (absolute value of values).\n"
      << "    -eigen_relative : Use eigen_relative differences as default tolerance type (absolute value of values).\n"
      << "    -eigen_combined : Use eigen_combined differences as default tolerance type (absolute value of values).\n"
      << "\n"
      << "    -show_all_diffs : Show all differences for all variables, not just the maximum\n"
      << "\n"
      << "    -m : Invokes a matching algorithm to create a mapping between the\n"
      << "         nodes and elements of the two files.  The topology must still be\n"
      << "         the same (within tolerance), but can be ordered differently.\n"
      << "\n"
      << "    -partial : Invokes a matching algorithm similar to the -m option.  However \n"
      << "               this option ignores unmatched nodes and elements.  This allows \n"
      << "               comparison of files that only partially overlap.\n"
      << "\n"
      << "    -match_ids : Invokes a matching algorithm using the node and element\n"
      << "                 global id maps in the two files.\n"
      << "\n"
      << "    -match_file_order : Invokes a matching algorithm using the node and element\n"
      << "                        position order in the two files.\n"
      << "\n"
      << "    -show_unmatched : If the -p switch is given, this prints out the \n"
      << "                      elements that did not match.\n"
      << "\n"
      << "    -dumpmap : If the -m switch is given, this prints out the resulting map.\n"
      << "\n"
      << "    -nsmap : Creates a map between the nodeset nodes in the two files\n"
      << "             if they include the same nodes, but are in different order.\n"
      << "\n"
      << "    -ssmap : Creates a map between the sideset faces in the two files\n"
      << "             if they include the same sides, but are in different order.\n"
      << "\n"
      << "    -no_nsmap : Compare nodeset nodes based on file order only\n"
      << "    -no_ssmap : Compare sideset faces based on file order only\n"
      << "\n"
      << "    -s : Short block type compare.  Forces element block type strings to\n"
      << "         be compared only up to the shortest string length.  For example,\n"
      << "         \"HEX\" and \"HEX8\" will be considered the same. (default)\n"
      << "\n"
      << "    -no_short : Do no do short block type compare.  Forces element block\n"
      << "                type strings to fully match. For example, \"HEX\" and \"HEX8\"\n"
      << "                will be considered different.\n"
      << "\n"
      << "    -ignore_case : Ignore case.  Variable names are compared case in-sensitive (default).\n"
      << "\n"
      << "    -case_sensitive : Variable names are compared case sensitive.\n"
      << "\n"
      << "    -ignore_maps: Output node and element diff summaries using file local implicit\n"
      << "                  ids instead of global ids.\n"
      << "\n"
      << "    -ignore_nans: Don't check data for NaNs\n"
      << "\n"
      << "    -ignore_dups: If two elements/nodes are in the same location in match or partial\n"
      << "                  match case, just return first match instead of aborting.\n"
      << "\n"
      << "    -ignore_attributes: Don't compare element attribute values.\n"
      << "\n"
      << "    -nosymm : No symmetric variable name checking.  By default, a warning will\n"
      << "              be produced if a name that is not to be excluded is contained\n"
      << "              in the second file given on the command line but not the first.\n"
      << "              This \"symmetric\" check can be turned off with this option\n"
      << "\n"
      << "    -allow_name_mismatch  Allow a variable name that is in the first database to\n"
      << "                          not be in the second database\n"
      << "\n"
      << "    -x <list> : Exclude time steps.  Does not consider the time steps\n"
      << "                given in the list of integers.  The format is comma\n"
      << "         separated and ranged integers (with no spaces), such as \"1,5-9,28\".\n"
      << "         The first time step is the number one.\n"
      << "\n"
      << "    -steps <beg:end:incr>: Specify subset of steps to consider. Syntax is beg:end:increment,\n"
      << "                           Enter -1:: for just the last step. If only beg set, end=beg\n"
      << "\n"
      << "    -norms: Calculate L2 norm of variable differences and output if > 0.0\n"
      << "\n"
      << "    -stat : Return exit status of 2 if the files are different.  Normally,\n"
      << "            the exit status is always zero unless an error occurs.\n"
      << "\n"
      << "    -maxnames <int>: There is a compiled limit of "
      << DEFAULT_MAX_NUMBER_OF_NAMES
      << " exodus names.\n"
      << "                     This option allows the maximum number to be set.\n"
      << "\n"
      << "    -use_old_floor: use the older defintion of the floor tolerance which was ignore if\n"
      << "                    |a-b| < floor.  The new definition is ignore if |a| < floor && |b| < floor."
      << "\n"
      << "    -summary : Produce a summary in exodiff input format.  This will\n"
      << "               create output with max/min statistics on the data in the\n"
      << "         format of an exodiff input file.  The algorithm to determine\n"
      << "         the minimum separation between any two nodes can be disabled with\n"
      << "         the \"no_coord_sep\" switch.\n"
      << "    -copyright : Output copyright and license information.\n"
      << "\n"
      << "    -f <cmd file> : Use the given file to specify the variables to be\n"
      << "                    be considered and to what tolerances.\n"
      << "                    Enter \"-H file\" for command file Syntax.\n"
      << std::endl;
  }
  if (option == "file" || option == "all") {
    std::cout
      << "\n  Command file syntax:\n"
      << "    -f <cmd file> : Use the given file to specify the variables to be\n"
      << "                    be considered and to what tolerances.\n"
      << "    Syntax.\n"
      << "\n"
      << "                # Anything following a # is a comment.\n"
      << "                DEFAULT TOLERANCE relative 1.E-8 floor 1.E-14\n"
      << "                COORDINATES absolute 1.E-12\n"
      << "                TIME STEPS absolute 1.E-14\n"
      << "                GLOBAL VARIABLES relative 1.E-4 floor 1.E-12\n"
      << "                NODAL VARIABLES absolute 1.E-8\n"
      << "                <tab> DISPLX\n"
      << "                <tab> VELX absolute 1.E-6\n"
      << "                <tab> VELY relative 1.E-6 floor 1.e-10\n"
      << "                ELEMENT VARIABLES\n"
      << "                <tab> !SIGYY\n"
      << "                <tab> !SIGZZ\n"
      << "\n"
      << "         - The variable names are case insensitive (unless or CASE SENSITIVE specified),\n"
      << "           All other comparisons are also case insensitive. Abreviations can be used. \n"
      << "         - All comparisons use the compiled default of relative 1.e-6 for\n"
      << "           variables and absolute 1.e-6 for coordinates.  This is overridden\n"
      << "           with the DEFAULT TOLERANCE line.  The DEFAULT TOLERANCE values\n"
      << "           are overridden by the values given on the VARIABLES line and apply\n"
      << "           only to those variables.  Each variable can override all values\n"
      << "           by following its name with a value.\n"
      << "         - A variable name must start with a tab character.  Only those\n"
      << "           variables listed will be considered.  The NOT symbol \"!\" means\n"
      << "           do not include this variable.  Mixing non-! and ! is not allowed\n"
      << "           without the \"(all)\" specifier, e.g.:\n\n"
      << "                NODAL VARIABLES (all) absolute 1.E-8\n"
      << "                <tab> DISPLX\n"
      << "                <tab> !VELX\n"
      << "                <tab> VELY relative 1.E-6 floor 1.e-10\n\n"
      << "           In this case, all variables are considered that are not prepended\n"
      << "           with a \"!\" symbol.\n"
      << "         - If a variable type (e.g. NODAL VARIABLES) is not specified, no\n"
      << "           variables of that type will be considered.\n"
      << "         - The command line option to set the maximum number of exodus \n"
      << "           names can be set with MAX NAMES <int>.  Note:  THIS OPTION MUST\n"
      << "           APPEAR BEFORE THE VARIABLE BLOCKS ARE READ!\n"
      << "         - The time step exclusion option can be used in the input file with\n"
      << "           the syntax \"EXCLUDE TIMES <list>\", where <list> has the same\n"
      << "           format as in the command line.\n"
      << "         - The matching algorithm, \"-m\", can be turned on from the input file\n"
      << "           with the APPLY MATCHING keyword on a separate line.\n"
      << "         - The nodeset matching algorithm, \"-nsmap\", can be turned on from the\n"
      << "           input file with the NODESET MATCH keyword on a separate line.\n"
      << "         - The sideset matching algorithm, \"-ssmap\", can be turned on from the\n"
      << "           input file with the SIDESET MATCH keyword on a separate line.\n"
      << "         - The short block type compare option, \"-s\", can be turned on with the\n"
      << "           SHORT BLOCKS keyword.\n"
      << "         - The no short compare option, \"-no_short\", can be turned on with the\n"
      << "           NO SHORT BLOCKS keyword.\n"
      << "         - The case_sensitive option, \"-case_sensitive\", can be turned on with the\n"
      << "           CASE SENSITIVE keyword.\n"
      << "         - The ignore case option, \"-i\", can be turned on with the\n"
      << "           IGNORE CASE keyword. (default behavior now)\n"
      << "         - The ignore maps option, \"-ignore_maps\", can be turned on with the\n"
      << "           IGNORE MAPS keyword.\n"
      << "         - The ignore nans option, \"-ignore_nans\", can be turned on with the\n"
      << "           IGNORE NANS keyword.\n"
      << "         - The ignore dups option, \"-ignore_dups\", can be turned on with the\n"
      << "           IGNORE DUPLICATES keyword.\n"
      << "         - The time step offset option, \"-T\", can be turned on with the \n"
      << "           STEP OFFSET keyword.\n"
      << "         - The automatic time step offset option, \"-TA\", can be turned\n"
      << "           on with the STEP OFFSET AUTOMATIC keyword.\n"
      << "         - The automatic time step offset option, \"-TM\", can be turned\n"
      << "           on with the STEP OFFSET MATCH keyword.\n"
      << "         - The calculation of the L2 norm of differences \"-norms\", can be turned\n"
      << "           on with the CALCULATE NORMS keyword.\n"
      << "         - The exit status return option, \"-stat\", can be turned on with the \n"
      << "           RETURN STATUS keyword.\n"
      << std::endl;
  }
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


  extern void add_to_log(const char *name);
  extern void Build_Variable_Names(ExoII_Read& file1, ExoII_Read& file2, bool *diff_found);
  extern bool Check_Global( ExoII_Read& file1, ExoII_Read& file2);

  extern void Check_Compatible_Meshes( ExoII_Read& file1,
				       ExoII_Read& file2,
				       bool check_only,
				       int *node_map,
				       int *elmt_map );

  int Create_File(ExoII_Read& file1, ExoII_Read& file2,
		  const string& diffile_name, bool *diff_found);
  double To_Double(const string & str_val);
  double FileDiff(double v1, double v2, TOLERANCE_TYPE_enum type);
  void Die_TS(double ts);

  int global_elmt_num(ExoII_Read& file, int b_idx, int e_idx);
  double Find_Min_Coord_Sep(ExoII_Read& file);
  int timeStepIsExcluded(int ts);
  const double *get_nodal_values( ExoII_Read &filen, int time_step,
				  int idx, int fno,
				  const string &name, bool *diff_flag );

  bool diff_globals( ExoII_Read& file1, ExoII_Read& file2,
		     int step1, int step2, int out_file_id,
		     MinMaxData *mm_glob, double *vals );
  bool diff_nodals( ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		    int out_file_id, int* node_map, const int *id_map,
		    MinMaxData *mm_node, double *vals);
  bool diff_element( ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		     int out_file_id, int* elmt_map, const int *id_map,
		     Exo_Block **blocks2, MinMaxData *mm_elmt,
		     double *vals );
  bool diff_element_attributes( ExoII_Read& file1, ExoII_Read& file2, int* elmt_map, const int *id_map,
				Exo_Block **blocks2);
  bool diff_nodeset( ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		     int out_file_id, const int *id_map, MinMaxData *mm_ns,
		     double *vals );
  bool diff_sideset( ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		     int out_file_id, const int *id_map, MinMaxData *mm_ss,
		     double *vals );

  void output_summary( ExoII_Read& file1, MinMaxData &mm_time,
		       MinMaxData *mm_glob, MinMaxData *mm_node,
		       MinMaxData *mm_elmt, MinMaxData *mm_ns, MinMaxData *mm_ss,
		       const int *node_id_map, const int *elem_id_map );


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
void output_init(ExoII_Read& file, int count, const char *prefix)
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
  }

  int main(int argc, char* argv[])
  {
    checking_invalid = false;
    invalid_data = false;
  
    if (sigfillset(&(sigact.sa_mask)) == -1) perror("sigfillset failed");
    sigact.sa_handler = floating_point_exception_handler;
    if (sigaction(SIGFPE, &sigact, 0) == -1) perror("sigaction failed");
#if defined(LINUX) && defined(GNU)
    // for GNU, this seems to be needed to turn on trapping
    feenableexcept(FE_DIVBYZERO | FE_OVERFLOW | FE_INVALID);
#endif
  
    specs.Set_Max_Names(DEFAULT_MAX_NUMBER_OF_NAMES);
  
    string file1_name, file2_name, diffile_name;
  
    Parse_Command_Line(argc, argv, file1_name, file2_name, diffile_name);
  
    if (specs.summary_flag && file1_name == "")
      {
	std::cout << "exodiff: ERROR: Summary option engaged but an exodus "
	  "file was not specified" << std::endl;
	exit(1);
      }
    if (specs.summary_flag) {
      file2_name = "";
      diffile_name = "";
      specs.glob_var_do_all_flag = true;
      specs.node_var_do_all_flag = true;
      specs.elmt_var_do_all_flag = true;
      specs.elmt_att_do_all_flag = true;
      specs.ns_var_do_all_flag   = true;
      specs.ss_var_do_all_flag   = true;
      specs.map_flag             = FILE_ORDER;
      specs.quiet_flag           = false;
    }
  
    if (!specs.quiet_flag && !specs.summary_flag) Print_Banner(" ");
    if (specs.summary_flag) Print_Banner("#");
  
    // Open input files.
    ExoII_Read file1(file1_name.c_str());
    ExoII_Read file2(file2_name.c_str());
    if (!specs.quiet_flag && !specs.summary_flag)
      std::cout << "Reading first file ... " << std::endl;
    string serr = file1.Open_File();
    if (!serr.empty()) {
      std::cout << "exodiff: " << serr << std::endl;
      exit(1);
    }
    if (!specs.summary_flag) {
      if (!specs.quiet_flag) std::cout << "Reading second file ... " << std::endl;
      serr = file2.Open_File();
      if (serr != "") {
	std::cout << "exodiff: " << serr << std::endl;
	exit(1);
      }
    }
  
    // Check that the maximum number of names has not been exceeded...
    if (file1.Num_Global_Vars() > specs.max_number_of_names ||
	file1.Num_Nodal_Vars()  > specs.max_number_of_names ||
	file1.Num_NS_Vars()     > specs.max_number_of_names ||
	file1.Num_SS_Vars()     > specs.max_number_of_names ||
	file1.Num_Elmt_Vars()   > specs.max_number_of_names) {
      int max = file1.Num_Global_Vars();
      if (file1.Num_Nodal_Vars() > max) max = file1.Num_Nodal_Vars();
      if (file1.Num_NS_Vars()    > max) max = file1.Num_NS_Vars();
      if (file1.Num_SS_Vars()    > max) max = file1.Num_SS_Vars();
      if (file1.Num_Elmt_Vars()  > max) max = file1.Num_Elmt_Vars();
      
      std::cout << "exodiff: Number of names in file 1 (" << max << ") is larger than "
		<< "current limit of " << specs.max_number_of_names
		<< ".  To increase, use \"-maxnames <int>\" on the command "
	"line or \"MAX NAMES <int>\" in the command file.  "
	"Aborting..."
		<< std::endl;
      exit(1);
    }

    // Check that the maximum number of names has not been exceeded...
    if (!specs.summary_flag) {
      if (file2.Num_Global_Vars() > specs.max_number_of_names ||
	  file2.Num_Nodal_Vars()  > specs.max_number_of_names ||
	  file2.Num_NS_Vars()     > specs.max_number_of_names ||
	  file2.Num_SS_Vars()     > specs.max_number_of_names ||
	  file2.Num_Elmt_Vars()   > specs.max_number_of_names) {
	int max = file2.Num_Global_Vars();
	if (file2.Num_Nodal_Vars() > max) max = file2.Num_Nodal_Vars();
	if (file2.Num_NS_Vars()    > max) max = file2.Num_NS_Vars();
	if (file2.Num_SS_Vars()    > max) max = file2.Num_SS_Vars();
	if (file2.Num_Elmt_Vars()  > max) max = file2.Num_Elmt_Vars();
	
	std::cout << "exodiff: Number of names in file 2 (" << max << ") is larger than "
		  << "current limit of " << specs.max_number_of_names
		  << ".  To increase, use \"-maxnames <int>\" on the command "
	  "line or \"MAX NAMES <int>\" in the command file.  "
	  "Aborting..."
		  << std::endl;
	exit(1);
      }
    }
    
    if (specs.summary_flag) {
      output_init(file1, 1, "#");
    } else {
      if (!specs.quiet_flag) {
	output_init(file1, 1, "");
	output_init(file2, 2, "");
	if (!specs.command_file_name.empty()) {
	  FileInfo fi(specs.command_file_name);
	  cout << "  COMMAND FILE: " << fi.realpath() << "\n\n";
	}
      }
    }
  
    if (!specs.summary_flag) {
      bool is_same = Check_Global(file1, file2);
      if (!is_same) {
	file1.Close_File();
	file2.Close_File();
	std::cout << "\nexodiff: Files are different" << std::endl;
	if (specs.exit_status_switch)
	  return 2;
	else
	  return 0;
      }
    }

    // When mapping is on ("-m"), node_map maps indexes from file1 to indexes
    // into file2.  Similarly with elmt_map.
    int *node_map = 0, *elmt_map = 0;
    if (specs.map_flag != FILE_ORDER) {
      if(specs.map_flag == PARTIAL) {
	Compute_Partial_Maps(node_map, elmt_map, file1, file2);
      } else if (specs.map_flag == USE_FILE_IDS) {
	Compute_FileId_Maps(node_map, elmt_map, file1, file2);
      } else if (specs.map_flag == DISTANCE) {
	Compute_Maps(node_map, elmt_map, file1, file2);
      } else {
	std::cout << "INTERNAL ERROR: Invalid map option." << std::endl;
      }

      if (specs.dump_mapping) {
	Dump_Maps(node_map, elmt_map, file1);
      }
      if (Check_Maps(node_map, elmt_map, file1, file2)) {
	if (specs.map_flag == DISTANCE) {
	  std::cout << "exodiff: INFO .. Map option is not needed.\n";
	}
	specs.map_flag = FILE_ORDER;
      }
    }  

    bool diff_flag = false; // Set to 'true' to indicate files contain diffs
    // Call this before checking for compatible meshes since it sets which variables
    // are going to be compared.  If no variables of a specific type, then not an error
    // if the meshes are different in that type.
    Build_Variable_Names(file1, file2, &diff_flag);

    int out_file_id = -1;
    if (!specs.summary_flag) {
      Check_Compatible_Meshes( file1, file2, (diffile_name == ""),
			       node_map, elmt_map );
      // Doesn't return if meshes are not compatible...
      
      out_file_id = Create_File(file1, file2, diffile_name, &diff_flag);
    }
  
    SMART_ASSERT( !( specs.summary_flag && out_file_id >= 0 ) );
  
    if (!specs.quiet_flag && !specs.summary_flag) {
      std::cout << "\n  ==============================================================\n";
      if (!specs.ignore_maps)
	std::cout << "  NOTE: All node and element ids are reported as global ids.\n\n";
      else
	std::cout << "  NOTE: All node and element ids are reported as local ids.\n\n";
    }
    else if (specs.summary_flag) {
      std::cout << "\n# ==============================================================\n";
      if (!specs.ignore_maps)
	std::cout << "#  NOTE: All node and element ids are reported as global ids.\n\n";
      else
	std::cout << "#  NOTE: All node and element ids are reported as local ids.\n\n";
    }
  
    double* var_vals = 0;
    if (out_file_id >= 0) {
      int max_ent = specs.glob_var_names->size();
      if (file1.Num_Nodes() > max_ent)
	max_ent = file1.Num_Nodes();
      if (file1.Num_Elmts() > max_ent)
	max_ent = file1.Num_Elmts();
    
      var_vals = new double[max_ent];
    }
 
    // When mapping is in effect, it is efficient to grab pointers to all blocks.
    Exo_Block** blocks2 = 0;
    if (elmt_map != 0) {
      blocks2 = new Exo_Block*[file2.Num_Elmt_Blocks()];
      for (int b = 0; b < file2.Num_Elmt_Blocks(); ++b)
	blocks2[b] = file2.Get_Elmt_Block_by_Index(b);
    }
  
    int min_num_times = file1.Num_Times();
  
    MinMaxData mm_time;   mm_time.type = MinMaxData::mm_time;
    MinMaxData *mm_glob = NULL;
    MinMaxData *mm_node = NULL;
    MinMaxData *mm_elmt = NULL;
    MinMaxData *mm_eatt = NULL;
    MinMaxData *mm_ns   = NULL;
    MinMaxData *mm_ss   = NULL;
  
    if (specs.summary_flag) {
      int n;
      if (specs.glob_var_names && (n = specs.glob_var_names->size()) > 0) {
	mm_glob = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_glob[i].type = MinMaxData::mm_global;
      }
      if (specs.node_var_names && (n = specs.node_var_names->size()) > 0) {
	mm_node = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_node[i].type = MinMaxData::mm_nodal;
      }
      if (specs.elmt_var_names && (n = specs.elmt_var_names->size()) > 0) {
	mm_elmt = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_elmt[i].type = MinMaxData::mm_element;
      }
      if (specs.elmt_att_names && (n = specs.elmt_att_names->size()) > 0) {
	mm_eatt = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_eatt[i].type = MinMaxData::mm_elematt;
      }
      if (specs.ns_var_names && (n = specs.ns_var_names->size()) > 0) {
	mm_ns = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_ns[i].type = MinMaxData::mm_nodeset;
      }
      if (specs.ss_var_names && (n = specs.ss_var_names->size()) > 0) {
	mm_ss = new MinMaxData[n];
	for (int i=0; i < n; i++) mm_ss[i].type = MinMaxData::mm_sideset;
      }
    }
    else {
      min_num_times = (file1.Num_Times() < file2.Num_Times()
		       ? file1.Num_Times() : file2.Num_Times());

      if (specs.time_step_stop > 0 && specs.time_step_stop < min_num_times) {
	min_num_times = specs.time_step_stop;
      }
    }
  
    if (specs.time_step_start < 0) {
      specs.time_step_start = min_num_times;
    }

    // If time_step_offset == -1, then determine the offset automatically.
    // Assumes file1 has more steps than file2 and that the last step(s) 
    // on file2 match the last step(s) on file1.
    if (specs.time_step_offset == -1) {
      specs.time_step_offset = file1.Num_Times() - file2.Num_Times();
      if (specs.time_step_offset < 0) {
	std::cout << "ERROR: Second database must have less timesteps than "
		  << "first database." << std::endl;
	exit(1);
      }
    }

    // If time_step_offset == -2, then determine the offset automatically.
    // Find the closest time on file1 to the first time on file2.
    // Assumes file1 has more steps than file2.
    if (specs.time_step_offset == -2) {
      if (file1.Num_Times() < file2.Num_Times()) {
	std::cout << "ERROR: Second database must have less timesteps than "
		  << "first database." << std::endl;
	exit(1);
      }

      double t2 = file2.Time(1);
      double maxdiff = fabs(t2 - file1.Time(1));
      int step = 1;
      for (int i=2; i < file1.Num_Times(); i++) {
	double t1 = file1.Time(i);
	double diff = fabs(t2 - t1);
	if (diff < maxdiff) {
	  step = i;
	  maxdiff = diff;
	}
      }
      specs.time_step_offset = step-1;
    }

    if (specs.time_step_offset > 0) {
      if (specs.time_step_start > 0) {
	std::cout << "The first " << specs.time_step_offset+specs.time_step_start-1
		  << " timesteps in the first database will be skipped because of time step offset and time step start settings.\n\n";
      } else {
	std::cout << "The first " << specs.time_step_offset
		  << " timesteps in the first database will be skipped because of time step offset setting.\n\n";
      }
    }

    // Get node and element number maps which map internal implicit ids into
    // global ids...
    // For now, assume that both files have the same map. At some point, need
    // to actually use the maps to build the correspondence map from one file
    // to the next...
    const int *node_id_map = NULL;
    const int *elem_id_map = NULL;
    if (!specs.ignore_maps) {
      file1.Load_Node_Map();
      file1.Load_Elmt_Map();
      node_id_map = file1.Get_Node_Map();
      elem_id_map = file1.Get_Elmt_Map();
      if (!specs.summary_flag)
	Compare_Maps(file1, file2, node_map, elmt_map, specs.map_flag == PARTIAL);
    } else {
      node_id_map = new int[file1.Num_Nodes()];
      int *tmp_map = const_cast<int*>(node_id_map);
      for (int i=0; i < file1.Num_Nodes(); i++) {
	tmp_map[i] = i+1;
      }
      elem_id_map = new int[file1.Num_Elmts()];
      tmp_map = const_cast<int*>(elem_id_map);
      for (int i=0; i < file1.Num_Elmts(); i++) {
	tmp_map[i] = i+1;
      }
    }
    
    // Diff attributes...
    if (!specs.ignore_attributes && elmt_map==NULL && !specs.summary_flag) {
      if (diff_element_attributes(file1, file2, elmt_map, elem_id_map, blocks2))
	diff_flag = true;
    }

    if (specs.time_step_start < 1) specs.time_step_start = 1;
    if (specs.time_step_start > min_num_times && min_num_times > 0) {
      std::cout << "\tERROR: Time step options resulted in no timesteps being compared\n";
      diff_flag = true;
    }
    for (int time_step = specs.time_step_start; time_step <= min_num_times;
	 time_step += specs.time_step_increment)
      {
	if (timeStepIsExcluded(time_step)) continue;
    
	int time_step1 = time_step + specs.time_step_offset;
	int time_step2 = time_step;
	SMART_ASSERT(time_step1 <= file1.Num_Times());
	if (!specs.summary_flag) {
	  SMART_ASSERT(time_step2 <= file2.Num_Times());
	}

	if (specs.summary_flag) {
	  double t = file1.Time(time_step1);
	  mm_time.spec_min_max(t, time_step1);
	}
	else if (out_file_id >= 0 && !specs.quiet_flag) {
	  std::cout << "Processing time step " << time_step1
		    << "  (Difference in time values = "
		    << (file1.Time(time_step1) - file2.Time(time_step2))
		    << ")" << std::endl;
	}
	else if (out_file_id < 0) {
	  if (!specs.quiet_flag) {
	    sprintf(buf, "  --------- Time step %d, %13.7e ~ %13.7e, %s diff: %12.5e",
		    time_step1, 
		    file1.Time(time_step1), file2.Time(time_step2),
		    specs.time_tol.abrstr(),
		    FileDiff(file1.Time(time_step1), file2.Time(time_step2), specs.time_tol.type));
	    std::cout << buf;;
	  }
      
	  if (specs.time_tol.Diff(file1.Time(time_step1), file2.Time(time_step2))) {
	    diff_flag = true;
	    if (specs.quiet_flag) Die_TS(time_step1);
	    else
	      std::cout << " (FAILED) " << std::endl;
	  }
	  else if (!specs.quiet_flag)
	    std::cout <<   " ---------" << std::endl;
	}
    
	if (out_file_id >= 0) {
	  double t = file1.Time(time_step1);
	  ex_put_time(out_file_id, time_step2, &t);
	}
    
	if ( diff_globals( file1, file2, time_step1, time_step2,
			   out_file_id, mm_glob, var_vals ) )
	  diff_flag = true;
     
	// Nodal variables.
	if ( diff_nodals( file1, file2, time_step1, time_step2,
			  out_file_id, node_map, node_id_map, mm_node, var_vals ) )
	  diff_flag = true;

	// Element variables.
	if ( diff_element( file1, file2, time_step1, time_step2, out_file_id,
			   elmt_map, elem_id_map, blocks2, mm_elmt, var_vals ) )
	  diff_flag = true;
        
	if (specs.map_flag != PARTIAL) {
	  // Nodeset variables.
	  if ( diff_nodeset( file1, file2, time_step1, time_step2, out_file_id,
			     node_id_map, mm_ns, var_vals ) )
	    diff_flag = true;
      
	  // Sideset variables.
	  if ( diff_sideset( file1, file2, time_step1, time_step2, out_file_id,
			     elem_id_map, mm_ss, var_vals ) )
	    diff_flag = true;
	} else {
	  if (specs.ns_var_names->size() > 0 || specs.ss_var_names->size() > 0) {
	    std::cout << "WARNING: nodeset and sideset variables not (yet) "
	      "compared for partial map\n";
	  }
	}
      }  // End of time step loop.
  
    // Make sure there is an operation to perform (compare times, variables, ...)
    if ((min_num_times == 0 && specs.coord_tol.type == IGNORE) ||
	(min_num_times >  0 && specs.time_tol.type == IGNORE &&
	 specs.glob_var_names->size() == 0 &&
	 specs.node_var_names->size() == 0 &&
	 specs.elmt_var_names->size() == 0 &&
	 specs.elmt_att_names->size() == 0 &&
	 specs.ns_var_names->size()   == 0 &&
	 specs.ss_var_names->size()   == 0)) {
      std::cout << "\nWARNING: No comparisons were performed during "
	"this execution.\n";
      diff_flag = true;
    }

    if (specs.summary_flag) {
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
      if (file1.Num_Times() - specs.time_step_offset == file2.Num_Times()) {
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
  
    if (!specs.ignore_maps) {
      file1.Free_Node_Map();
      file1.Free_Elmt_Map();
    } else {
      delete [] node_id_map;
      delete [] elem_id_map;
    }

    if (var_vals       != 0) delete [] var_vals;
    if (blocks2        != 0) delete [] blocks2;
    if (node_map       != 0) delete [] node_map;
    if (elmt_map       != 0) delete [] elmt_map;
  
    delete [] mm_glob;
    delete [] mm_node;
    delete [] mm_elmt;
    delete [] mm_ns;
    delete [] mm_ss;
  
    file1.Close_File();
    if (!specs.summary_flag)
      file2.Close_File();

#if 1
    add_to_log(argv[0]);
#else
    // Temporarily differentiate this version from previous version in logs.
    ostringstream code;
    code << "exodiff-" << Version();
    add_to_log( code.str().c_str() );
#endif
  
    if (specs.exit_status_switch && diff_flag)
      return 2;
    else
      return 0;
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
    if (specs.exit_status_switch)
      exit(2);
    else
      exit(1);
  }

  int global_elmt_num(ExoII_Read& file, int b_idx, int e_idx)
  {
    SMART_ASSERT(b_idx < file.Num_Elmt_Blocks());
  
    int g = 0;
    for (int b = 0; b < file.Num_Elmt_Blocks(); ++b)
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

  bool Invalid_Values(const double *values, int count)
  {
    bool valid = true;
    if (!specs.ignore_nans) {
      checking_invalid = true;
      invalid_data = false;
    
      SMART_ASSERT(values != NULL);
    
    
      for (int i=0; i < count; i++) {
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

  const double *get_nodal_values(ExoII_Read &filen, int time_step, int idx,
				 int fno, const string &name, bool *diff_flag)
    {
      const double *vals = NULL;
      if (fno == 1 || !specs.summary_flag) {
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

  bool diff_globals(ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		    int out_file_id, MinMaxData *mm_glob, double *gvals)
  {
    bool diff_flag = false;
  
    // Global variables.
    file1.Load_Global_Results(step1);
    const double* vals1 = file1.Get_Global_Results();
    const double* vals2 = 0;
    if (!specs.summary_flag) {
      file2.Load_Global_Results(step2);
      vals2 = file2.Get_Global_Results();
    }
    
    // ----------------------------------------------------------------------
    // Output file containing differences...
    if (out_file_id >= 0) {
      if (specs.glob_var_names->size() > 0) {
	SMART_ASSERT(gvals != 0);
	for (unsigned out_idx = 0; out_idx < specs.glob_var_names->size(); ++out_idx) {
	  const string& name = (*specs.glob_var_names)[out_idx];
	  int idx1 = find_string(file1.Global_Var_Names(), name, specs.nocase_var_names);
	  int idx2 = find_string(file2.Global_Var_Names(), name, specs.nocase_var_names);
	  SMART_ASSERT(idx1 >= 0 && idx2 >= 0);
	  gvals[out_idx] = FileDiff(vals1[idx1], vals2[idx2], specs.output_type);
	}
	ex_put_glob_vars(out_file_id, step2, specs.glob_var_names->size(), gvals);
      }
      return diff_flag;
    }

    // --------------------------------------------------------------------
    // Summary output 
    if (specs.summary_flag) {
      for (unsigned out_idx = 0; out_idx < specs.glob_var_names->size(); ++out_idx) {
	const string& name = (*specs.glob_var_names)[out_idx];
	int idx1 = find_string(file1.Global_Var_Names(), name, specs.nocase_var_names);
	SMART_ASSERT(idx1 >= 0);
      
	mm_glob[out_idx].spec_min_max(vals1[idx1], step1);
      }
      return diff_flag;
    }

    // -------------------------------------------------------------------
    // Determine if any diffs and output to terminal
    int name_length = max_string_length(file1.Global_Var_Names())+1;
    if (!specs.quiet_flag && specs.glob_var_names->size() > 0)
      std::cout << "Global variables:" << std::endl;
    
    for (unsigned out_idx = 0; out_idx < specs.glob_var_names->size(); ++out_idx) {
      const string& name = (*specs.glob_var_names)[out_idx];
      int idx1 = find_string(file1.Global_Var_Names(), name, specs.nocase_var_names);
      int idx2 = find_string(file2.Global_Var_Names(), name, specs.nocase_var_names);
      SMART_ASSERT(idx1 >= 0 && idx2 >= 0);
      
      if (Invalid_Values(&vals1[idx1], 1)) {
	std::cout << "\tERROR: NaN found for variable " << name << " in file 1\n";
	diff_flag = true;
      }

      if (Invalid_Values(&vals2[idx2], 1)) {
	std::cout << "\tERROR: NaN found for variable " << name << " in file 2\n";
	diff_flag = true;
      }

      if (specs.glob_var[out_idx].Diff(vals1[idx1], vals2[idx2])) {
	diff_flag = true;
          
	if (!specs.quiet_flag) {
	  sprintf(buf, "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (FAILED)",
		  name_length, 
		  name.c_str(), specs.glob_var[out_idx].abrstr(),
		  vals1[idx1], vals2[idx2],
		  specs.glob_var[out_idx].Delta(vals1[idx1], vals2[idx2]) );
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(step1);
      }
    }
    return diff_flag;
  }

  bool diff_nodals(ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		   int out_file_id, int *node_map, const int *id_map,
		   MinMaxData *mm_node, double *nvals)
  {
    bool diff_flag = false;

    // ---------------------------------------------------------------------
    // Output file containing differences...
    if (out_file_id >= 0) {
      SMART_ASSERT(nvals != 0);
      for (unsigned n_idx = 0; n_idx < specs.node_var_names->size(); ++n_idx) {
	const string& name = (*specs.node_var_names)[n_idx];
	int idx1 = find_string(file1.Nodal_Var_Names(), name, specs.nocase_var_names);
	int idx2 = find_string(file2.Nodal_Var_Names(), name, specs.nocase_var_names);
	SMART_ASSERT(idx1 >= 0 && idx2 >= 0);
      
	const double* vals1 = get_nodal_values(file1, step1, idx1, 1,
					       name, &diff_flag);
	const double* vals2 = get_nodal_values(file2, step2, idx2, 2,
					       name, &diff_flag);
      
	int ncount = file1.Num_Nodes();
	for (int n = 0; n < ncount; ++n) {
        
	  // Should this node be processed...
	  if (node_map == 0 || node_map[n]>=0){
	    int n2 = node_map != 0 ? node_map[n] : n;
	    nvals[n] = FileDiff(vals1[n], vals2[n2], specs.output_type);
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
    if (specs.summary_flag) {
      for (unsigned n_idx = 0; n_idx < specs.node_var_names->size(); ++n_idx) {
	const string& name = (*specs.node_var_names)[n_idx];
	int idx1 = find_string(file1.Nodal_Var_Names(), name, specs.nocase_var_names);
	SMART_ASSERT(idx1 >= 0);
	const double* vals1 = get_nodal_values(file1, step1, idx1, 1,
					       name, &diff_flag);

	int ncount = file1.Num_Nodes();
	for (int n = 0; n < ncount; ++n) {
	  mm_node[n_idx].spec_min_max(vals1[n], step1, n);
	} 
      }
      file1.Free_Nodal_Results();
      return diff_flag;
    }

    SMART_ASSERT(!specs.summary_flag && out_file_id <0);
    // ----------------------------------------------------------------------
    // Determine if any diffs and output to terminal
    if (!specs.quiet_flag && specs.node_var_names->size() > 0)
      std::cout << "Nodal variables:" << std::endl;
    int name_length = max_string_length(file1.Nodal_Var_Names())+1;
    
    for (unsigned n_idx = 0; n_idx < specs.node_var_names->size(); ++n_idx) {
      const string& name = (*specs.node_var_names)[n_idx];
      int idx1 = find_string(file1.Nodal_Var_Names(), name, specs.nocase_var_names);
      int idx2 = find_string(file2.Nodal_Var_Names(), name, specs.nocase_var_names);
      SMART_ASSERT(idx1 >= 0 && idx2 >= 0);
      
      const double* vals1 = get_nodal_values(file1, step1, idx1, 1,
					     name, &diff_flag);
      const double* vals2 = get_nodal_values(file2, step2, idx2, 2,
					     name, &diff_flag);

      DiffData max_diff;
      double norm_d = 0.0;
      double norm_1 = 0.0;
      double norm_2 = 0.0;
      int ncount = file1.Num_Nodes();
      for (int n = 0; n < ncount; ++n) {

	// Should this node be processed...
	if (node_map == 0 || node_map[n]>=0){
	  int n2 = node_map != 0 ? node_map[n] : n;
	  double d = specs.node_var[n_idx].Delta(vals1[n], vals2[n2]);
	  if (specs.show_all_diffs) {
	    if (d > specs.node_var[n_idx].value) {
	      diff_flag = true;
	      sprintf(buf, "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (node %d)",
		      name_length,
		      name.c_str(), specs.node_var[n_idx].abrstr(),
		      vals1[n], vals2[n], d, id_map[n]);
	      std::cout << buf << std::endl;
	    }
	  } else {
	    max_diff.set_max(d, vals1[n], vals2[n2], n);
	  }
	  if (specs.doNorms) {
	    norm_d += (vals1[n]-vals2[n2])*(vals1[n]-vals2[n2]);
	    norm_1 += vals1[n]*vals1[n];
	    norm_2 += vals2[n]*vals2[n];
	  }
	}
      } // End of node iteration...

      if (specs.doNorms && norm_d > 0.0) {
	norm_d = sqrt(norm_d);
	norm_1 = sqrt(norm_1);
	norm_2 = sqrt(norm_2);
        sprintf(buf,
                "   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e) rel=%14.7e",
                name_length, name.c_str(),
		norm_d, norm_1, norm_2, norm_d / max(norm_1, norm_2));
        std::cout << buf << std::endl;
      }

      if (max_diff.diff > specs.node_var[n_idx].value) {
	diff_flag = true;
	if (!specs.quiet_flag) {
	  sprintf(buf, "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (node %d)",
		  name_length,
		  name.c_str(), specs.node_var[n_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, id_map[max_diff.id]);
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

  bool diff_element(ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		    int out_file_id, int* elmt_map, const int *id_map,
		    Exo_Block **blocks2, MinMaxData *mm_elmt, double *evals)
  {
    bool diff_flag = false;
    
    if (out_file_id >= 0) {SMART_ASSERT(evals != 0);}
  
    if (out_file_id < 0 && !specs.quiet_flag && !specs.summary_flag && specs.elmt_var_names->size() > 0)
      std::cout << "Element variables:" << std::endl;
    
    int name_length = max_string_length(*specs.elmt_var_names)+1;

    for (unsigned e_idx = 0; e_idx < specs.elmt_var_names->size(); ++e_idx) {
      const string& name = (*specs.elmt_var_names)[e_idx];
      int vidx1 = find_string(file1.Elmt_Var_Names(), name, specs.nocase_var_names);
      int vidx2 = 0;
      if (!specs.summary_flag)
	vidx2 = find_string(file2.Elmt_Var_Names(), name, specs.nocase_var_names);
      SMART_ASSERT(vidx1 >= 0 && vidx2 >= 0);
      
      double norm_d = 0.0;
      double norm_1 = 0.0;
      double norm_2 = 0.0;
    
      if (elmt_map != 0) { // Load variable for all blocks in file 2.
	for (int b = 0; b < file2.Num_Elmt_Blocks(); ++b) {
	  Exo_Block * block2 = file2.Get_Elmt_Block_by_Index(b);
	  block2->Load_Results(step2, vidx2);
	}
      }
      
      int global_elmt_index = 0;
      DiffData max_diff;
      int e2;
      for (int b = 0; b < file1.Num_Elmt_Blocks(); ++b) {
	Exo_Block* eblock1 = file1.Get_Elmt_Block_by_Index(b);
	if (!eblock1->is_valid_var(vidx1)) {
	  global_elmt_index += eblock1->Size();
	  continue;
	}
        
	Exo_Block* eblock2 = NULL;
	int b2 = b;
	if (elmt_map == 0 && !specs.summary_flag) {
	  int id = eblock1->Id();
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
      
	if (elmt_map == 0 && !specs.summary_flag) {
	  // Without mapping, get result for this block.
	  int id = eblock1->Id();
	  eblock2 = file2.Get_Elmt_Block_by_Id(id);
	  eblock2->Load_Results(step2, vidx2);
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
        
	int ecount =  eblock1->Size();
	int block_id = eblock1->Id();
	for (int e = 0; e < ecount; ++e) {
	  if (out_file_id >= 0)evals[e] = 0.;
	  int el_flag = 1;
	  if(elmt_map != 0)
	    el_flag = elmt_map[global_elmt_index];

	  if(el_flag >= 0){
	    if (!specs.summary_flag) {
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
          
	    if (specs.summary_flag) {
	      mm_elmt[e_idx].spec_min_max(vals1[e], step1,
					  global_elmt_index, block_id);
	    }
	    else if (out_file_id >= 0) {
	      evals[e] = FileDiff(vals1[e], v2, specs.output_type);
	    }
	    else if (specs.show_all_diffs) {
	      double d = specs.elmt_var[e_idx].Delta(vals1[e], v2);
	      if (d > specs.elmt_var[e_idx].value) {
		diff_flag = true;
		sprintf(buf,
			"   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (block %d, elmt %d)",
			name_length, name.c_str(),
			specs.elmt_var[e_idx].abrstr(),
			vals1[e], v2, d, block_id, id_map[global_elmt_index]);
		std::cout << buf << std::endl;
	      }
	    }
	    else {
	      double d = specs.elmt_var[e_idx].Delta(vals1[e], v2);
	      max_diff.set_max(d, vals1[e], v2, global_elmt_index, block_id);
	    }
	    if (specs.doNorms) {
	      norm_d += (vals1[e]-v2)*(vals1[e]-v2);
	      norm_1 += vals1[e]*vals1[e];
	      norm_2 += v2*v2;
	    }
	  }
	  ++global_elmt_index;
	}
        
	if (out_file_id >= 0)
	  ex_put_var(out_file_id, step2, EX_ELEM_BLOCK, e_idx+1,
		     eblock1->Id(), eblock1->Size(), evals);
        
	eblock1->Free_Results();
	if (!specs.summary_flag && elmt_map == 0) {
	  eblock2->Free_Results();
	}
        
      }  // End of element block loop.
    
      if (specs.doNorms && norm_d > 0.0) {
	norm_d = sqrt(norm_d);
	norm_1 = sqrt(norm_1);
	norm_2 = sqrt(norm_2);
	sprintf(buf,
                "   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e) rel=%14.7e",
                name_length, name.c_str(),
		norm_d, norm_1, norm_2, norm_d / max(norm_1, norm_2));
        std::cout << buf << std::endl;
      }
      
      if (!specs.summary_flag && max_diff.diff > specs.elmt_var[e_idx].value) {
	diff_flag = true;
        
	if (!specs.quiet_flag) {
	  sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (block %d, elmt %d)",
		  name_length, name.c_str(),
		  specs.elmt_var[e_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, max_diff.blk, id_map[max_diff.id]);
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(step1);
      }

    }  // End of element variable loop.
    return diff_flag;
  }

  bool diff_nodeset(ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		    int out_file_id, const int *id_map,
		    MinMaxData *mm_ns, double *vals)
  {
    string serr;
    bool diff_flag = false;
  
    if (out_file_id >= 0) {SMART_ASSERT(vals != 0);}
  
    int name_length = max_string_length(file1.NS_Var_Names())+1;
      
    if ( out_file_id < 0 && !specs.quiet_flag &&
	 !specs.summary_flag && specs.ns_var_names->size() > 0 )
      std::cout << "Nodeset variables:" << std::endl;
    
    for (unsigned e_idx = 0; e_idx < specs.ns_var_names->size(); ++e_idx) {
      const string& name = (*specs.ns_var_names)[e_idx];
      int vidx1 = find_string(file1.NS_Var_Names(), name, specs.nocase_var_names);
      int vidx2 = 0;
      if (!specs.summary_flag)
	vidx2 = find_string(file2.NS_Var_Names(), name, specs.nocase_var_names);
      SMART_ASSERT(vidx1 >= 0 && vidx2 >= 0);
      
      double norm_d = 0.0;
      double norm_1 = 0.0;
      double norm_2 = 0.0;

      DiffData max_diff;
      for (int b = 0; b < file1.Num_Node_Sets(); ++b) {
	Node_Set* nset1 = file1.Get_Node_Set_by_Index(b);
	if (!nset1->is_valid_var(vidx1)) {
	  continue;
	}
        
	Node_Set* nset2 = NULL;
	if (!specs.summary_flag) {
	  int id = nset1->Id();
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
	if (!specs.summary_flag) {
	  // Without mapping, get result for this nset
	  nset2->Load_Results(step2, vidx2);
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
        
	int ncount = nset1->Size();
	for (int e = 0; e < ncount; ++e) {
	  int idx1 = nset1->Node_Index(e);
	  int idx2 = 0;

	  if (out_file_id >= 0) vals[idx1] = 0.;
	  if (!specs.summary_flag) {
	    idx2 = nset2->Node_Index(e);
	    v2 = vals2[idx2];
	  }
          
	  if (specs.summary_flag) {
	    mm_ns[e_idx].spec_min_max(vals1[idx1], step1, e, nset1->Id());
	  }
	  else if (out_file_id >= 0) {
	    vals[idx1] = FileDiff(vals1[idx1], v2, specs.output_type);
	  }
	  else if (specs.show_all_diffs) {
	    double d = specs.ns_var[e_idx].Delta(vals1[idx1], v2);
	    if (d > specs.ns_var[e_idx].value) {
	      diff_flag = true;
	      sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (set %d, node %d)",
		  name_length, name.c_str(), specs.ns_var[e_idx].abrstr(),
		      vals1[idx1], v2, d, nset1->Id(), e);
	      std::cout << buf << std::endl;
	    }
	  }
	  else {
	    double d = specs.ns_var[e_idx].Delta(vals1[idx1], v2);
	    max_diff.set_max(d, vals1[idx1], v2, e, nset1->Id());
	  }
	  if (specs.doNorms) {
	    norm_d += (vals1[idx1]-v2)*(vals1[idx1]-v2);
	    norm_1 += vals1[idx1]*vals1[idx1];
	    norm_2 += v2*v2;
	  }
	}
        
	if (out_file_id >= 0)
	  ex_put_var(out_file_id, step2, EX_NODE_SET,
		     e_idx+1, nset1->Id(),
		     nset1->Size(), vals);
        
	nset1->Free_Results();
	if (!specs.summary_flag) {
	  nset2->Free_Results();
	}
        
      }  // End of nodeset loop.
      
      if (specs.doNorms && norm_d > 0.0) {
        sprintf(buf,
                "   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e)",
                name_length, name.c_str(),
		sqrt(norm_d), sqrt(norm_1), sqrt(norm_2));
        std::cout << buf << std::endl;
      }
      
      if (!specs.summary_flag && max_diff.diff > specs.ns_var[e_idx].value) {
	diff_flag = true;
        
	if (!specs.quiet_flag) {
	  Node_Set *nset = file1.Get_Node_Set_by_Id(max_diff.blk);
	  sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (set %d, node %d)",
		  name_length,
		  name.c_str(),
		  specs.ns_var[e_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, max_diff.blk,
		  id_map[nset->Node_Id(max_diff.id)-1]);
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(step1);
      }
    }  // End of nodeset variable loop.
    return diff_flag;
  }

  bool diff_sideset(ExoII_Read& file1, ExoII_Read& file2, int step1, int step2,
		    int out_file_id, const int *id_map,
		    MinMaxData *mm_ss, double *vals)
  {
    string serr;
    bool diff_flag = false;
  
    if (out_file_id >= 0) {SMART_ASSERT(vals != 0);}
  
    int name_length = max_string_length(file1.SS_Var_Names())+1;
      
    if (out_file_id < 0 && !specs.quiet_flag && !specs.summary_flag && specs.ss_var_names->size() > 0)
      std::cout << "Sideset variables:" << std::endl;
    
    double norm_d = 0.0;
    double norm_1 = 0.0;
    double norm_2 = 0.0;

    for (unsigned e_idx = 0; e_idx < specs.ss_var_names->size(); ++e_idx) {
      const string& name = (*specs.ss_var_names)[e_idx];
      int vidx1 = find_string(file1.SS_Var_Names(), name, specs.nocase_var_names);
      int vidx2 = 0;
      if (!specs.summary_flag)
	vidx2 = find_string(file2.SS_Var_Names(), name, specs.nocase_var_names);
      SMART_ASSERT(vidx1 >= 0 && vidx2 >= 0);
      
      DiffData max_diff;
      for (int b = 0; b < file1.Num_Side_Sets(); ++b) {
	Side_Set* sset1 = file1.Get_Side_Set_by_Index(b);
	SMART_ASSERT(sset1 != NULL);	
	if (!sset1->is_valid_var(vidx1)) {
	  continue;
	}

	Side_Set* sset2 = NULL;
	if (!specs.summary_flag) {
	  int id = sset1->Id();
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
	if (!specs.summary_flag) {
	  sset2->Load_Results(step2, vidx2);
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
        
	int ecount = sset1->Size();
	for (int e = 0; e < ecount; ++e) {
	  int ind1 = sset1->Side_Index(e);
	  int ind2 = 0;
	  if (out_file_id >= 0) vals[ind1] = 0.;
	  if (!specs.summary_flag) {
	    ind2 = sset2->Side_Index(e);
	    v2 = vals2[ind2];
	  }
          
	  if (specs.summary_flag) {
	    mm_ss[e_idx].spec_min_max(vals1[ind1], step1, e, sset1->Id());
	  }
	  else if (out_file_id >= 0) {
	    vals[ind1] = FileDiff(vals1[ind1], v2, specs.output_type);
	  }
	  else if (specs.show_all_diffs) {
	    double d = specs.ss_var[e_idx].Delta(vals1[ind1], v2);
	    if (d > specs.ss_var[e_idx].value) {
	      diff_flag = true;
	      sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (set %d, side %d.%d)",
		      name_length, name.c_str(), specs.ss_var[e_idx].abrstr(),
		      vals1[ind1], v2, d,
		      sset1->Id(),
		      id_map[sset1->Side_Id(e).first-1],
		      sset1->Side_Id(e).second);
	      std::cout << buf << std::endl;
	    }
	  }
	  else {
	    double d = specs.ss_var[e_idx].Delta(vals1[ind1], v2);
	    max_diff.set_max(d, vals1[ind1], v2, e, sset1->Id());
	  }
	  if (specs.doNorms) {
	    norm_d += (vals1[ind1]-v2)*(vals1[ind1]-v2);
	    norm_1 += vals1[ind1]*vals1[ind1];
	    norm_2 += v2*v2;
	  }
	}
        
	if (out_file_id >= 0)
	  ex_put_var(out_file_id, step2, EX_SIDE_SET,
		     e_idx+1, sset1->Id(),
		     sset1->Size(), vals);
	sset1->Free_Results();
	if (!specs.summary_flag) {
	  sset2->Free_Results();
	}
      }  // End of sideset loop.
      
      if (!specs.summary_flag && max_diff.diff > specs.ss_var[e_idx].value) {
	diff_flag = true;
        
	if (specs.doNorms && norm_d > 0.0) {
	  sprintf(buf,
		  "   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e)",
		  name_length, name.c_str(),
		  sqrt(norm_d), sqrt(norm_1), sqrt(norm_2));
	  std::cout << buf << std::endl;
	}
      
	if (!specs.quiet_flag) {
	  Side_Set *sset = file1.Get_Side_Set_by_Id(max_diff.blk);
	  sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (set %d, side %d.%d)",
		  name_length,
		  name.c_str(),
		  specs.ss_var[e_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, max_diff.blk,
		  id_map[sset->Side_Id(max_diff.id).first-1],
		  sset->Side_Id(max_diff.id).second);
	  std::cout << buf << std::endl;
	}
	else
	  Die_TS(step1);
      }

    }  // End of sideset variable loop.
    return diff_flag;
  }

bool diff_element_attributes(ExoII_Read& file1, ExoII_Read& file2, 
			     int* elmt_map, const int *id_map,
			     Exo_Block **blocks2)
{
  bool diff_was_output = false;
  bool diff_flag = false;
    
  int global_elmt_offset = 0;
  for (int b = 0; b < file1.Num_Elmt_Blocks(); ++b) {
    Exo_Block* eblock1 = file1.Get_Elmt_Block_by_Index(b);
    SMART_ASSERT(eblock1 != NULL);
	
    int block_id = eblock1->Id();
    
    Exo_Block *eblock2 = file2.Get_Elmt_Block_by_Id(block_id);
    SMART_ASSERT(eblock2 != NULL);

    if (!diff_was_output && (eblock1->attr_count() > 0 || eblock2->attr_count() > 0)) {
      diff_was_output = true;
      std::cout << "Element attributes:" << std::endl;
    }      

    int name_length = max_string_length(eblock1->Attribute_Names())+1;
    for (int idx1=0; idx1 < eblock1->attr_count(); idx1++) {
      int global_elmt_index = global_elmt_offset;
      
      DiffData max_diff;
      const std::string &name = eblock1->Get_Attribute_Name(idx1);
	  
      // Find same attribute in eblock2...
      int idx2 = eblock2->Find_Attribute_Index(name);
      if (idx2 < 0) {
	continue;
      }

      // Find name in specs.elmt_att_names
      int tol_idx = -1;
      for (unsigned e_idx = 0; e_idx < specs.elmt_att_names->size(); ++e_idx) {
	if (name == (*specs.elmt_att_names)[e_idx]) {
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
        
      int ecount =  eblock1->Size();
      for (int e = 0; e < ecount; ++e) {
          
	if (specs.show_all_diffs) {
	  double d = specs.elmt_att[tol_idx].Delta(vals1[e], vals2[e]);
	  if (d > specs.elmt_att[tol_idx].value) {
	    diff_flag = true;
	    sprintf(buf,
		    "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (block %d, elmt %d)",
		    name_length, name.c_str(),
		    specs.elmt_att[tol_idx].abrstr(),
		    vals1[e], vals2[e], d, block_id, id_map[global_elmt_index]);
	    std::cout << buf << std::endl;
	  }
	}
	else {
	  double d = specs.elmt_att[tol_idx].Delta(vals1[e], vals2[e]);
	  max_diff.set_max(d, vals1[e], vals2[e], global_elmt_index, block_id);
	}
	if (specs.doNorms) {
	  norm_d += (vals1[e]-vals2[e])*(vals1[e]-vals2[e]);
	  norm_1 += vals1[e]*vals1[e];
	  norm_2 += vals2[e]*vals2[e];
	}
	++global_elmt_index;
      }
        
      if (specs.doNorms && norm_d > 0.0) {
	norm_d = sqrt(norm_d);
	norm_1 = sqrt(norm_1);
	norm_2 = sqrt(norm_2);
	sprintf(buf,
		"   %-*s L2 norm of diff=%14.7e (%11.5e ~ %11.5e) rel=%14.7e",
		name_length, name.c_str(),
		norm_d, norm_1, norm_2, norm_d / max(norm_1, norm_2));
	std::cout << buf << std::endl;
      }
      
      if (!specs.summary_flag && max_diff.diff > specs.elmt_att[tol_idx].value) {
	diff_flag = true;
        
	if (!specs.quiet_flag) {
	  sprintf(buf,
		  "   %-*s %s diff: %14.7e ~ %14.7e =%12.5e (block %d, elmt %d)",
		  name_length, name.c_str(),
		  specs.elmt_att[tol_idx].abrstr(),
		  max_diff.val1, max_diff.val2,
		  max_diff.diff, max_diff.blk, id_map[max_diff.id]);
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

  void output_summary(ExoII_Read& file1, MinMaxData &mm_time, MinMaxData *mm_glob,
		      MinMaxData *mm_node, MinMaxData *mm_elmt,
		      MinMaxData *mm_ns, MinMaxData *mm_ss,
		      const int *node_id_map, const int *elem_id_map)
  {
    int name_length = 0;
    int i, n;

    std::cout << "# NOTES:  - The min/max values are reporting the min/max "
	      << "in absolute value.\n"
	      << "#         - Time values (t) are 1-offset time step numbers.\n"
	      << "#         - Element block numbers are the block ids.\n"
	      << "#         - Node(n) and element(e) numbers are 1-offset."
	      << std::endl;
    
    if (specs.coord_sep) {
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

    n = specs.glob_var_names->size();
    if (n > 0) {
      std::cout << "GLOBAL VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(*specs.glob_var_names);
      for (i = 0; i < n; ++i) {
	sprintf(buf, "\t%-*s  # min: %15.8g @ t%d\tmax: %15.8g @ t%d\n",
		name_length, ((*specs.glob_var_names)[i]).c_str(),
		mm_glob[i].min_val, mm_glob[i].min_step,
		mm_glob[i].max_val, mm_glob[i].max_step);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No GLOBAL VARIABLES\n";
    }
    
    n = specs.node_var_names->size();
    if (n > 0) {
      std::cout << std::endl << "NODAL VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(*specs.node_var_names);
      for (i = 0; i < n; ++i) {
	sprintf(buf, "\t%-*s  # min: %15.8g @ t%d,n%d\tmax: %15.8g @ t%d,n%d\n",
		name_length, ((*specs.node_var_names)[i]).c_str(),
		mm_node[i].min_val, mm_node[i].min_step,
		node_id_map[mm_node[i].min_id],
		mm_node[i].max_val, mm_node[i].max_step,
		node_id_map[mm_node[i].max_id]);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No NODAL VARIABLES\n";
    }
    
    n = specs.elmt_var_names->size();
    if (n > 0) {
      std::cout << std::endl << "ELEMENT VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(*specs.elmt_var_names);
      for (i = 0; i < n; ++i) {
	sprintf(buf, "\t%-*s  # min: %15.8g @ t%d,b%d,e%d\tmax: %15.8g @ t%d,b%d,e%d\n",
		name_length, ((*specs.elmt_var_names)[i]).c_str(),
		mm_elmt[i].min_val, mm_elmt[i].min_step,
		mm_elmt[i].min_blk, elem_id_map[mm_elmt[i].min_id],
		mm_elmt[i].max_val, mm_elmt[i].max_step,
		mm_elmt[i].max_blk, elem_id_map[mm_elmt[i].max_id]);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No ELEMENT VARIABLES\n";
    }

    n = specs.ns_var_names->size();
    if (n > 0) {
      std::cout << std::endl << "NODESET VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(*specs.ns_var_names);
      for (i = 0; i < n; ++i) {
	Node_Set *nsmin = file1.Get_Node_Set_by_Id(mm_ns[i].min_blk);
	Node_Set *nsmax = file1.Get_Node_Set_by_Id(mm_ns[i].max_blk);
	sprintf(buf,
		"\t%-*s  # min: %15.8g @ t%d,s%d,n%d\tmax: %15.8g @ t%d,s%d,n%d\n",
		name_length, ((*specs.ns_var_names)[i]).c_str(),
		mm_ns[i].min_val, mm_ns[i].min_step, mm_ns[i].min_blk,
		node_id_map[nsmin->Node_Id(mm_ns[i].min_id)-1],
		mm_ns[i].max_val, mm_ns[i].max_step, mm_ns[i].max_blk, 
		node_id_map[nsmax->Node_Id(mm_ns[i].max_id)-1]);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No NODESET VARIABLES\n";
    }
  
    n = specs.ss_var_names->size();
    if (n > 0) {
      std::cout << std::endl << "SIDESET VARIABLES relative 1.e-6 floor 0.0\n";
      name_length = max_string_length(*specs.ss_var_names);
      for (i = 0; i < n; ++i) {
	Side_Set *ssmin = file1.Get_Side_Set_by_Id(mm_ss[i].min_blk);
	Side_Set *ssmax = file1.Get_Side_Set_by_Id(mm_ss[i].max_blk);
	std::pair<int,int> min_side = ssmin->Side_Id(mm_ss[i].min_id);
	std::pair<int,int> max_side = ssmax->Side_Id(mm_ss[i].max_id);
	sprintf(buf,
		"\t%-*s  # min: %15.8g @ t%d,s%d,f%d.%d\tmax: %15.8g @ t%d,s%d,f%d.%d\n",
		name_length, ((*specs.ss_var_names)[i]).c_str(),
		mm_ss[i].min_val, mm_ss[i].min_step, mm_ss[i].min_blk,
		elem_id_map[min_side.first-1], min_side.second,
		mm_ss[i].max_val, mm_ss[i].max_step, mm_ss[i].max_blk,
		elem_id_map[max_side.first-1], max_side.second);
	std::cout << buf;
      }
    } else {
      std::cout << "\n# No SIDESET VARIABLES\n";
    }
    std::cout << std::endl;
  }

  int timeStepIsExcluded(int ts)
  {
    for (int i = 0; i < specs.num_excluded_steps; ++i)
      if (ts == specs.exclude_steps[i])
	return 1;
  
    return 0;
  }
