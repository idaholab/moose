#include "ED_SystemInterface.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <vector>

#include <limits.h>
#include <cstdlib>
#include <cstring>

#include "stringx.h"
#include "ED_Version.h"
#include <SL_tokenize.h>

namespace {
  void Parse_Die(const char* line)
  {
    std::string sline = line;
    chop_whitespace(sline);
    std::cout << "exodiff: Error parsing input file, currently at \""
	      << sline << "\"." << std::endl;
    exit(1);
  }


  std::string Parse_Variables(std::string xline, std::ifstream& cmd_file,
			      bool& all_flag, Tolerance &def_tol,
			      std::vector<std::string>& names,
			      std::vector<Tolerance> &toler,
			      int max_names);

  int case_strcmp(const std::string &s1, const std::string &s2)
  {
    const char *c1 = s1.c_str();
    const char *c2 = s2.c_str();
    for ( ; ; c1++, c2++) {
      if (std::tolower(*c1) != std::tolower(*c2))
	return (std::tolower(*c1) - std::tolower(*c2));
      if (*c1 == '\0')
	return 0;
    }
  }
  void file_help();
  void tolerance_help();

  double To_Double(const std::string & str_val)
  {
    SMART_ASSERT(str_val.size() > 0);

    char* endptr; errno = 0;
    double val = strtod(str_val.c_str(), &endptr);

    if (errno == ERANGE) {
      std::cerr << "exodiff: ERROR:  Overflow or underflow occured when trying"
		<< " to parse command line tolerance.  Aborting..." << std::endl;
      exit(1);
    }
    errno = 0;

    if (val < 0.0) {
      std::cerr << "exodiff: ERROR:  Parsed a negative value \""
		<< val << "\".  Aborting..." << std::endl;
      exit(1);
    }
    return val;
  }

  int File_Exists(const std::string & fname)
  {
    if (fname.empty()) return 0;
    std::ifstream file_check(fname.c_str(), std::ios::in);
    if (file_check.fail()) return 0;
    file_check.close();
    return 1;
  }

  void Parse_Steps_Option(const std::string &option, int &start, int &stop, int &increment)
  {
    //: The defined formats for the count attribute are:<br>
    //:  <ul>
    //:    <li><missing> -- default -- 1 <= count <= oo  (all steps)</li>
    //:    <li>"X"                  -- X <= count <= X  (just step X)</li>
    //:    <li>"X:Y"                -- X to Y by 1</li>
    //:    <li>"X:"                 -- X to oo by 1</li>
    //:    <li>":Y"                 -- 1 to Y by 1</li>
    //:    <li>"::Z"                -- 1 to oo by Z</li>
    //:  </ul>
    //: The count and step must always be >= 0

    // Break into tokens separated by ":"

    // Default is given in constructor above...

    if (option == "last" || option == "LAST") {
      start = -1;
      return;
    }
    const char *tokens = option.c_str();
    if (tokens != NULL) {
      if (strchr(tokens, ':') != NULL) {
	// The string contains a separator

	int vals[3];
	vals[0] = start;
	vals[1] = stop;
	vals[2] = increment;

	int j=0;
	for (int i=0; i < 3; i++) {
	  // Parse 'i'th field
	  char tmp_str[128];;
	  int k=0;

	  while (tokens[j] != '\0' && tokens[j] != ':') {
	    tmp_str[k++] = tokens[j++];
	  }

	  tmp_str[k] = '\0';
	  if (strlen(tmp_str) > 0)
	    vals[i] = strtol(tmp_str, NULL, 0);

	  if (tokens[j++] == '\0') {
	    break; // Reached end of string
	  }
	}
	start     = vals[0];
	stop      = vals[1];
	increment = vals[2];
      } else {
	// Does not contain a separator, min == max
	start = stop = strtol(tokens, NULL, 0);
      }
    }
  }

  void Check_Parsed_Names(std::vector<std::string>& names, bool& all_flag)
  {
    int num_include = 0;
    int num_exclude = 0;
    for (unsigned i = 0; i < names.size(); ++i) {
      SMART_ASSERT(names[i] != "");
      if (names[i][0] == '!')
	++num_exclude;
      else
	++num_include;
    }
    if (!all_flag && num_include > 0 && num_exclude > 0) {
      std::cerr << "exodiff: ERROR: Parsing error: Cannot specify both "
	"variables to include and exclude without using the "
	"'(all)' specifier.  Aborting..." << std::endl;
      exit(1);
    }
    if (num_include == 0 && num_exclude > 0) all_flag = true;
  }


  void parseExcludeTimes(std::string exclude_arg, std::vector<int> &exclude_steps)
  {
    std::string arg_copy = exclude_arg;

    int num_excluded_steps = 0;

    // first pass just counts the number of excluded time steps:

    std::string tok = extract_token( exclude_arg, "," );
    while (tok.size() > 0)
      {
	std::string subtok = extract_token( tok, "-" );
	SMART_ASSERT(subtok.size() > 0);

	errno = 0;
	int ival1 = atoi( subtok.c_str() );  SMART_ASSERT(errno == 0);

	if (ival1 < 1) {
	  std::cerr << "exodiff: Error parsing exclusion times from command "
	    "line .. value was less than 1" << std::endl;
	  exit(1);
	}

	++num_excluded_steps;

	subtok = extract_token( tok, "-" );
	if (subtok.size() > 0)
	  {
	    errno = 0;
	    int ival2 = atoi( subtok.c_str() );  SMART_ASSERT(errno == 0);

	    if (ival2 < 1) {
	      std::cerr << "exodiff: Error parsing exclusion times from command "
		"line .. value was less than 1" << std::endl;
	      exit(1);
	    }

	    if (ival1 < ival2) {
	      for (int i = ival1+1; i <= ival2; ++i) ++num_excluded_steps;
	    }
	    else if (ival1 > ival2) {
	      std::cerr << "exodiff: Error parsing exclusion times from command "
		"line .. first value in a range was greater than the "
		"second" << std::endl;
	      exit(1);
	    }
	  }

	tok = extract_token( exclude_arg, "," );
      }

    if (num_excluded_steps > 0)
      {
	exclude_steps.resize(num_excluded_steps);

	// second pass collects the excluded time steps

	exclude_arg = arg_copy;
	num_excluded_steps = 0;

	tok = extract_token( exclude_arg, "," );
	while (tok.size() > 0)
	  {
	    std::string subtok = extract_token( tok, "-" );
	    SMART_ASSERT(subtok.size() > 0);

	    errno = 0;
	    int ival1 = atoi( subtok.c_str() );  SMART_ASSERT(errno == 0);

	    exclude_steps[num_excluded_steps++] = ival1;

	    subtok = extract_token( tok, "-" );
	    if (subtok.size() > 0)
	      {
		errno = 0;
		int ival2 = atoi( subtok.c_str() );  SMART_ASSERT(errno == 0);

		for (int i = ival1+1; i <= ival2; ++i)
		  exclude_steps[num_excluded_steps++] = i;
	      }

	    tok = extract_token( exclude_arg, "," );
	  }
      }
  }
}

SystemInterface::SystemInterface()
  : quiet_flag           (false),
    show_all_diffs       (false),
    output_type          (ABSOLUTE),
    map_flag             (USE_FILE_IDS),
    nsmap_flag           (true),
    ssmap_flag           (true),
    short_block_check    (true),
    nocase_var_names     (true),
    summary_flag         (false),
    ignore_maps          (false),
    ignore_nans          (false),
    ignore_dups          (false),
    ignore_attributes    (false),
    ints_64_bits         (false),
    coord_sep            (false),
    exit_status_switch   (true),
    dump_mapping         (false),
    show_unmatched       (false),
    noSymmetricNameCheck (false),
    allowNameMismatch    (false),
    doNorms              (false),
    pedantic             (false),
    interpolating        (false),
    coord_tol            ( ABSOLUTE, 1.0e-6, 0.0 ),
    time_tol             ( RELATIVE, 1.0e-6, 1.0e-15 ),
    final_time_tol       ( RELATIVE, 0.0,    0.0 ),
    time_step_offset     (0),
    time_step_start      (1),
    time_step_stop       (-1),
    time_step_increment  (1),
    max_number_of_names  (DEFAULT_MAX_NUMBER_OF_NAMES),
    default_tol          ( RELATIVE, 1.0e-6, 0.0 ),
    glob_var_do_all_flag (false),
    node_var_do_all_flag (false),
    elmt_var_do_all_flag (false),
    elmt_att_do_all_flag (false),
    ns_var_do_all_flag   (false),
    ss_var_do_all_flag   (false),
    command_file("")
{
  glob_var_default = default_tol;
  node_var_default = default_tol;
  elmt_var_default = default_tol;
  elmt_att_default = default_tol;
  ns_var_default = default_tol;
  ss_var_default = default_tol;

  enroll_options();
}

SystemInterface::~SystemInterface() {}

void SystemInterface::show_version()
{
  std::cout << "EXODIFF\t(Version: " << version << ") Modified: " << verdate << '\n';
}

void SystemInterface::enroll_options()
{
  options_.usage("[options] file1.exo file2.exo [diffile.exo]\n"
		 "\tor:  exodiff -summary <file.exo> (create variable summary)  \n"
		 "\tor:  exodiff [-help]             (usage)                    \n"
		 "\tor:  exodiff [-version]\n");

  options_.enroll("help", GetLongOption::OptionalValue,
		  "Print this summary and exit.\n"
		  "\t\tEnter \"-help file\" for the syntax of the command file\n"
		  "\t\t      \"-help tolerance\" for information on the supported tolerance options.",
		  0, "usage");

  options_.enroll("Help", GetLongOption::NoValue,
		  "Print this summary and exit.\n", 0);

  options_.enroll("tolerance", GetLongOption::MandatoryValue,
		  "Overrides the default tolerance of 1.0E-6.",
		  "1.0E-6");

  options_.enroll("Floor", GetLongOption::MandatoryValue,
		  "Overrides the default floor tolerance of 0.0.",
		  "0.0");

  options_.enroll("TimeStepOffset", GetLongOption::MandatoryValue,
		  "Timestep 'x+offset' in first file matches timestep 'x' in second file.",
		  0);
  options_.enroll("TA", GetLongOption::NoValue,
		 "Automatic determination of timestep offset -- end at same step.", 0);
  options_.enroll("TM", GetLongOption::NoValue,
		  "Automatic determination of timestep offset -- closest match to first step on file2.", 0);
  options_.enroll("interpolate", GetLongOption::NoValue,
		  "Interpolate times on file2 to match times on file1.", 0);
  options_.enroll("final_time_tolerance", GetLongOption::MandatoryValue,
		  "Tolerance on matching of final times on database when interpolate option specified\n."
		  "\t\tIf final times do not match within this tolerance, files are different.", 0);
  options_.enroll("quiet", GetLongOption::NoValue,
		  "Quiet.  Only errors will be sent to stdout.  Comparison mode will echo\n"
		  "\t\t\"exodiff: Files are the same.\" or \"exodiff: Files are different.\"", 0);

  // Tolerance type options...
  options_.enroll("absolute", GetLongOption::NoValue,
		  "Default tolerance is absolute difference. |a-b| > tolerance", 0);
  options_.enroll("relative", GetLongOption::NoValue,
		  "Default tolerance is relative difference. |a-b| > max(|a|,|b|)*tolerance", 0);
  options_.enroll("combined", GetLongOption::NoValue,
		  "Default tolerance is combined difference. (-help tolerance for info)", 0);
  options_.enroll("ulps_float", GetLongOption::NoValue,
		  "Default tolerance if number of ulps (units last position) of difference\n"
		  "\t\twhen values converted to floats.", 0);
  options_.enroll("ulps_double", GetLongOption::NoValue,
		  "Default tolerance is number of ulps (units last position) of difference.", 0);
  options_.enroll("eigen_absolute", GetLongOption::NoValue,
		  "Default tolerance is absolute differences of the absolute value of the values.", 0);
  options_.enroll("eigen_relative", GetLongOption::NoValue,
		  "Default tolerance is relative differences of the absolute value of the values.", 0);
  options_.enroll("eigen_combined", GetLongOption::NoValue,
		  "Default tolerance is combined differences of the absolute value of the values.", 0);
  options_.enroll("ignore", GetLongOption::NoValue,
		  "Default tolerance is ignored (turn off all checking by default).", 0);

  options_.enroll("show_all_diffs", GetLongOption::NoValue,
		  "Show all differences for all variables, not just the maximum.", 0);

  options_.enroll("map", GetLongOption::NoValue,
		  "Invokes a matching algorithm to create a mapping between the\n"
		  "\t\tnodes and elements of the two files.  The topology must still be\n"
		  "\t\tthe same (within tolerance), but can be ordered differently.", 0);
  options_.enroll("partial", GetLongOption::NoValue,
		  "Invokes a matching algorithm similar to the -m option.  However \n"
		  "\t\tthis option ignores unmatched nodes and elements.  This allows \n"
		  "\t\tcomparison of files that only partially overlap.", 0);
  options_.enroll("match_ids", GetLongOption::NoValue,
		  "Invokes a matching algorithm using the node and element global id\n"
		  "\t\tmaps in the two files.", 0);
  options_.enroll("match_file_order", GetLongOption::NoValue,
		  "Invokes a matching algorithm using the node and element position\n"
		  "\t\torder in the two files.", 0);
  options_.enroll("show_unmatched", GetLongOption::NoValue,
		  "If the -partial switch is given, this prints out the elements that did not match.", 0);
  options_.enroll("dumpmap", GetLongOption::NoValue,
		  "If the -map switch is given, this prints out the resulting map.", 0);
  options_.enroll("nsmap", GetLongOption::NoValue,
		  "Creates a map between the nodeset nodes in the two files\n"
		  "\t\tif they include the same nodes, but are in different order.", 0);
  options_.enroll("ssmap", GetLongOption::NoValue,
		  "Creates a map between the sideset faces in the two files\n"
		  "\t\tif they include the same sides, but are in different order.", 0);
  options_.enroll("no_nsmap", GetLongOption::NoValue,
		  "Compare nodeset nodes based on file order only", 0);
  options_.enroll("no_ssmap", GetLongOption::NoValue,
		  "Compare sideset faces based on file order only", 0);
  options_.enroll("pedantic", GetLongOption::NoValue,
		  "Be more picky about what is a difference.", 0);
  options_.enroll("short", GetLongOption::NoValue,
		  "Short block type compare.  Forces element block type strings to\n"
		  "\t\tbe compared only up to the shortest string length.  For example,\n"
		  "\t\t\"HEX\" and \"HEX8\" will be considered the same. (default)", 0);
  options_.enroll("no_short", GetLongOption::NoValue,
		  "Do not do short block type compare.  Forces element block\n"
		  "\t\ttype strings to fully match. For example, \"HEX\" and \"HEX8\"\n"
		  "\t\twill be considered different.", 0);
  options_.enroll("ignore_case", GetLongOption::NoValue,
		  "Ignore case.  Variable names are compared case in-sensitive (default).", 0);
  options_.enroll("case_sensitive", GetLongOption::NoValue,
		  "Variable names are compared case sensitive.", 0);
  options_.enroll("ignore_maps", GetLongOption::NoValue,
		  "Output node and element diff summaries using file local implicit ids\n"
		  "\t\tinstead of global ids.", 0);
  options_.enroll("ignore_nans", GetLongOption::NoValue,
		  "Don't check data for NaNs", 0);
  options_.enroll("ignore_dups", GetLongOption::NoValue,
		  "If two elements/nodes are in the same location in match or partial\n"
		  "                  match case, just return first match instead of aborting.", 0);
  options_.enroll("ignore_attributes", GetLongOption::NoValue,
		  "Don't compare element attribute values.", 0);
  options_.enroll("64-bit", GetLongOption::NoValue,
		  "True if forcing the use of 64-bit integers for the output file",
		  NULL);
  options_.enroll("nosymmetric_name_check", GetLongOption::NoValue,
		  "No symmetric variable name checking.  By default, a warning will\n"
		  "\t\tbe produced if a name that is not to be excluded is contained\n"
		  "\t\tin the second file given on the command line but not the first.\n"
		  "\t\tThis \"symmetric\" check can be turned off with this option.", 0);
  options_.enroll("allow_name_mismatch", GetLongOption::NoValue,
		  "Allow a variable name that is in the first database to not be in the\n"
		  "\t\tsecond database", 0);
  options_.enroll("x", GetLongOption::MandatoryValue,
		  "Exclude time steps.  Does not consider the time steps given in the list of integers.\n"
		  "\t\tThe format is comma-separated and ranged integers (with no spaces), such as \"1,5-9,28\".\n"
		  "\t\tThe first time step is the number '1'.", 0);
  options_.enroll("exclude", GetLongOption::MandatoryValue,
		  "Exclude time steps.  Does not consider the time steps given in the list of integers.\n"
		  "\t\tThe format is comma-separated and ranged integers (with no spaces), such as \"1,5-9,28\".\n"
		  "\t\tThe first time step is the number '1'.", 0);
  options_.enroll("steps",  GetLongOption::MandatoryValue,
		  "Specify subset of steps to consider. Syntax is beg:end:increment,\n"
		  "\t\tEnter '-steps last' for just the last step. If only beg set, end=beg", 0);
  options_.enroll("explicit",  GetLongOption::MandatoryValue,
		  "Specify an explicit match of a step on database 1 with a step on database 2.\n"
		  "\t\tSyntax is '-explicit db1_step:db2_step' where 'db*_step' is either\n"
		  "\t\tthe 1-based step number or 'last' for the last step on the database.\n"
		  "\t\tExample: '-explicit 42:last' to match step 42 on database 1 with last step on database 2", 0);
  options_.enroll("norms", GetLongOption::NoValue,
		  "Calculate L2 norm of variable differences and output if > 0.0", 0);
  options_.enroll("status", GetLongOption::NoValue,
		  "Return exit status of 2 if the files are different. (default).", 0);
  options_.enroll("ignore_status", GetLongOption::NoValue,
		  "The exit status is always zero unless an error occurs.", 0);
  options_.enroll("maxnames", GetLongOption::MandatoryValue,
		  "There is a compiled limit of 1000 exodus names.\n"
		  "\t\tThis option allows the maximum number to be changed.",
		  "1000");
  options_.enroll("use_old_floor", GetLongOption::NoValue,
		  "use the older defintion of the floor tolerance.\n"
		  "\t\tOLD: ignore if |a-b| < floor.\n"
		  "\t\tNEW: ignore if |a| < floor && |b| < floor.", 0);
  options_.enroll("summary", GetLongOption::NoValue,
		  "Produce a summary in exodiff input format.\n"
		  "\t\tThis will create output with max/min statistics on the data in the format\n"
		  "\t\tof an exodiff input file.", 0);
  options_.enroll("min_coordinate_separation", GetLongOption::NoValue,
		  "In summary mode, calculate the minimum distance between any two nodes", 0);
  options_.enroll("copyright", GetLongOption::NoValue,
		  "Output copyright and license information.", 0);
  options_.enroll("version", GetLongOption::NoValue,
		  "Output code version", 0);

  options_.enroll("file", GetLongOption::MandatoryValue,
		  "Use the given file to specify the variables to be considered and to\n"
		  "\t\twhat tolerances. Enter \"-help file\" for the syntax of the command file", 0);
  options_.enroll("m", GetLongOption::NoValue,
		  "Backward-compatible option for -map", 0);
  options_.enroll("p", GetLongOption::NoValue,
		  "Backward-compatible option for -partial.", 0);
  options_.enroll("s", GetLongOption::NoValue,
		  "Backward-compatible option for -short", 0);
  options_.enroll("i", GetLongOption::NoValue,
		  "Backward-compatible option for -ignore_case.", 0);
  options_.enroll("f", GetLongOption::MandatoryValue,
		  "Backward-compatible option for -file", 0);
  options_.enroll("T", GetLongOption::MandatoryValue,
		  "Backward-compatible option for -TimeStepOffset", 0);
}

void SystemInterface::Set_Max_Names(int size)
{
  max_number_of_names = size;
  glob_var.resize(max_number_of_names, default_tol);
  node_var.resize(max_number_of_names, default_tol);
  elmt_var.resize(max_number_of_names, default_tol);
  elmt_att.resize(max_number_of_names, default_tol);
  ns_var.resize(max_number_of_names, default_tol);
  ss_var.resize(max_number_of_names, default_tol);
}

bool SystemInterface::parse_options(int argc, char **argv)
{
  int option_index = options_.parse(argc, argv);
  if ( option_index < 1 )
    return false;

  {
    const char *temp = options_.retrieve("help");
    if (temp) {
      if ((case_strcmp("usage", temp) == 0) || (case_strcmp("all", temp) == 0)) {
	options_.usage();
      }
      if ((case_strcmp("file", temp) == 0) || (case_strcmp("all", temp) == 0)) {
	file_help();
      }
      if ((case_strcmp("tolerance", temp) == 0) || (case_strcmp("all", temp) == 0)) {
	tolerance_help();
      }
      std::cerr << "\n\t\tCan also set options via EXODIFF_OPTIONS environment variable.\n";
      std::cerr << "\t\t->->-> Send email to gdsjaar@sandia.gov for exodiff support.<-<-<-\n";
      exit(EXIT_SUCCESS);
    }
  }

  if (options_.retrieve("Help")) {
    options_.usage();
    exit(EXIT_SUCCESS);
  }

  if (options_.retrieve("version")) {
    show_version();
    exit(EXIT_SUCCESS);
  }

  if (options_.retrieve("copyright")) {
    std::cerr << "\n"
	      << "Copyright(C) 2008 Sandia Corporation.\n"
	      << "\n"
	      << "Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,\n"
	      << "the U.S. Government retains certain rights in this software.\n"
	      << "        \n"
	      << "Redistribution and use in source and binary forms, with or without\n"
	      << "modification, are permitted provided that the following conditions are\n"
	      << "met:\n"
	      << "\n"
	      << "    * Redistributions of source code must retain the above copyright\n"
	      << "      notice, this list of conditions and the following disclaimer.\n"
	      << "\n"
	      << "    * Redistributions in binary form must reproduce the above\n"
	      << "      copyright notice, this list of conditions and the following\n"
	      << "      disclaimer in the documentation and/or other materials provided\n"
	      << "      with the distribution.\n"
	      << "    * Neither the name of Sandia Corporation nor the names of its\n"
	      << "      contributors may be used to endorse or promote products derived\n"
	      << "      from this software without specific prior written permission.\n"
	      << "\n"
	      << "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
	      << "'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
	      << "LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
	      << "A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n"
	      << "OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n"
	      << "SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n"
	      << "LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
	      << "DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
	      << "THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
	      << "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
	      << "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n";
    exit(EXIT_SUCCESS);
  }

  // Parse remaining options as filenames
  if (option_index < argc) {
    file1 = argv[option_index++];
    if (option_index < argc) {
      file2 = argv[option_index++];
    }
    if (option_index < argc) {
      if (option_index+1 == argc) {
	diff_file = argv[option_index++];
      } else {
	// Check for additional unknown arguments...
	std::cerr << "\nERROR: Too many file arguments specified."
		  << "\n       Probably options following filenames which is no longer allowed."
		  << "\n       Unknown options are: ";
	while (option_index < argc) {
	  std::cerr << "'" << argv[option_index++] << "' ";
	}
	std::cerr << "\n\n";
	return false;
      }
    }
  } else {
    std::cerr << "\nERROR: no files specified\n\n";
    return false;
  }

  // Get options from environment variable also...
  char *options = getenv("EXODIFF_OPTIONS");
  if (options != NULL) {
    std::cerr << "\nThe following options were specified via the EXODIFF_OPTIONS environment variable:\n"
	      << "\t\t" << options << "\n\n";
    options_.parse(options, options_.basename(*argv));
  }

  if (options_.retrieve("summary")) {
    summary_flag = true;
  }

  if (options_.retrieve("min_coordinate_separation")) {
    coord_sep = true;
  }

  {
    const char *temp = options_.retrieve("exclude");
    if (temp) parseExcludeTimes(temp, exclude_steps);
  }

  {
    const char *temp = options_.retrieve("x");
    if (temp) parseExcludeTimes(temp, exclude_steps);
  }

  {
    const char *temp = options_.retrieve("tolerance");
    if (temp) {
      default_tol.value = To_Double(temp);
    }
  }

  {
    const char *temp = options_.retrieve("Floor");
    if (temp) {
      default_tol.floor = To_Double(temp);
    }
  }

  {
    const char *temp = options_.retrieve("TimeStepOffset");
    if (temp) {
      errno = 0;
      time_step_offset =  atoi(temp);  SMART_ASSERT(errno == 0);
    } else {
      const char *temp2 = options_.retrieve("T");
      if (temp2) {
	errno = 0;
	time_step_offset =  atoi(temp2);  SMART_ASSERT(errno == 0);
      }
    }
  }

  if (options_.retrieve("TA")) {
    time_step_offset =  -1; // Signifies automatic offset calculation.
  }

  if (options_.retrieve("TM")) {
    time_step_offset =  -2; // Signifies automatic offset calculation -- closest match
  }

  {
    const char *temp = options_.retrieve("steps");
    if (temp) {
      Parse_Steps_Option(temp, time_step_start, time_step_stop, time_step_increment);
    }
  }

  {
    const char *temp = options_.retrieve("explicit");
    if (temp) {
      // temp should be of the form <ts1>:<ts2>  where ts# is either a timestep number
      // (1-based) or 'last'
      std::vector<std::string> tokens;
      SLIB::tokenize(temp, ":", tokens);
      if (tokens.size() == 2) {
	if (case_strcmp(tokens[0], "last") == 0) {
	  explicit_steps.first = -1;
	} else {
	  // Try to convert to integer...
	  explicit_steps.first = strtol(tokens[0].c_str(), NULL, 0);
	}

	if (case_strcmp(tokens[1], "last") == 0) {
	  explicit_steps.second = -1;
	} else {
	  // Try to convert to integer...
	  explicit_steps.second = strtol(tokens[1].c_str(), NULL, 0);
	}
      }
      else {
	std::cout << "exodiff: ERROR: parse error for -explicit keyword. "
	  "Expected '<int|last>:<int|last>', found '" << temp << "' Aborting..." << std::endl;
	exit(1);
      }
    }
  }

  if (options_.retrieve("quiet")) {
    quiet_flag = true;
  }

  if (options_.retrieve("show_all_diffs")) {
    show_all_diffs = true;
  }

  if (options_.retrieve("partial") || options_.retrieve("p")) {
    map_flag = PARTIAL;
  }

  if (options_.retrieve( "match_ids")) {
    map_flag = USE_FILE_IDS;
  }

  if (options_.retrieve("match_file_order")) {
    map_flag = FILE_ORDER;
  }

  if (options_.retrieve("map") || options_.retrieve("m")) {
    map_flag = DISTANCE;
  }
  if (options_.retrieve("nsmap")) {
    nsmap_flag = true;
  }
  if (options_.retrieve("no_nsmap")) {
    nsmap_flag = false;
  }
  if (options_.retrieve("ssmap")) {
    ssmap_flag = true;
  }
  if (options_.retrieve("no_ssmap")) {
    ssmap_flag = false;
  }
  if (options_.retrieve( "short") || options_.retrieve( "s")) {
    short_block_check = true;
  }
  if (options_.retrieve("no_short")) {
    short_block_check = false;
  }
  if (options_.retrieve("nosymmetric_name_check")) {
    noSymmetricNameCheck = true;
  }
  if (options_.retrieve("norms")) {
    doNorms = true;
  }
  if (options_.retrieve("pedantic")) {
    pedantic = true;
  }
  if (options_.retrieve( "interpolate")) {
    interpolating = true;
  }

  {
    const char *temp = options_.retrieve( "final_time_tolerance");
    if (temp) {
      final_time_tol.value = To_Double(temp);
    }
  }

  if (options_.retrieve("allow_name_mismatch")) {
    allowNameMismatch = true;
  }
  if (options_.retrieve("ignore_case") || options_.retrieve("i")) {
    nocase_var_names = true;
  }
  if (options_.retrieve("case_sensitive")) {
    nocase_var_names = false;
  }
  if (options_.retrieve("ignore_maps")) {
    ignore_maps = true;
  }
  if (options_.retrieve("ignore_nans")) {
    ignore_nans = true;
  }
  if (options_.retrieve("ignore_dups")) {
    ignore_dups = true;
  }
  if (options_.retrieve("64-bit")) {
    ints_64_bits = true;
  }
  if (options_.retrieve("ignore_attributes")) {
    ignore_attributes = true;
  }
  if (options_.retrieve("relative")) {
    output_type      = RELATIVE;  // Change type to relative.
    default_tol.type = RELATIVE;
  }
  if (options_.retrieve("ignore")) {
    output_type      = IGNORE;  // Change type to ignored
    default_tol.type = IGNORE;
  }
  if (options_.retrieve("absolute")) {
    output_type      = ABSOLUTE;  // Change type to absolute
    default_tol.type = ABSOLUTE;
  }
  if (options_.retrieve("combined")) {
    output_type      = COMBINED;  // Change type to combine
    default_tol.type = COMBINED;
  }
  if (options_.retrieve("ulps_float")) {
    output_type      = ULPS_FLOAT;
    default_tol.type = ULPS_FLOAT;
  }
  if (options_.retrieve("ulps_double")) {
    output_type      = ULPS_DOUBLE;
    default_tol.type = ULPS_DOUBLE;
  }
  if (options_.retrieve("eigen_relative")) {
    output_type      = EIGEN_REL;  // Change type to relative.
    default_tol.type = EIGEN_REL;
  }
  if (options_.retrieve("eigen_absolute")) {
    output_type      = EIGEN_ABS;  // Change type to absolute
    default_tol.type = EIGEN_ABS;
  }
  if (options_.retrieve("eigen_combined")) {
    output_type      = EIGEN_COM;  // Change type to combine
    default_tol.type = EIGEN_COM;
  }
  if (options_.retrieve("dumpmap")) {
    dump_mapping = true;
  }
  if (options_.retrieve("show_unmatched")) {
    show_unmatched = true;
  }

  {
    const char *temp = options_.retrieve("maxnames");
    if (temp) {
      errno = 0;
      int tmp = atoi(temp);  SMART_ASSERT(errno == 0);
      if (tmp > 0) Set_Max_Names(tmp);
    }
  }

  if (options_.retrieve( "status")) {
    exit_status_switch = true;
  }

  if (options_.retrieve( "ignore_status")) {
    exit_status_switch = false;
  }

  if (options_.retrieve("use_old_floor")) {
    Tolerance::use_old_floor = true;  // Change type to relative.
  }

  {
    // Reset default tolerances in case the -t flag was given.
    time_tol         = default_tol;
    glob_var_default = default_tol;
    node_var_default = default_tol;
    elmt_var_default = default_tol;
    elmt_att_default = default_tol;
    ns_var_default   = default_tol;
    ss_var_default   = default_tol;

    const char *temp = options_.retrieve("file");
    if (temp) {
      command_file = temp;
      if (!summary_flag && !File_Exists(command_file)) {
	std::cerr << "exodiff: Can't open file \"" << command_file << "\"." << std::endl;
	exit(1);
      }

      // Command file exists, parse contents...
      Parse_Command_File();
    }
    else {
      const char *t2 = options_.retrieve("f");
      if (t2) {
	command_file = t2;
	if (!summary_flag && !File_Exists(command_file)) {
	  std::cerr << "exodiff: Can't open file \"" << command_file << "\"." << std::endl;
	  exit(1);
	}

	// Command file exists, parse contents...
	Parse_Command_File();
      }
      else {
	glob_var_do_all_flag = true;
	node_var_do_all_flag = true;
	elmt_var_do_all_flag = true;
	elmt_att_do_all_flag = true;
	ns_var_do_all_flag = true;
	ss_var_do_all_flag = true;
      }
    }
  }
  return true;
}

void SystemInterface::Parse_Command_File()
{
  int default_tol_specified = 0;

  std::ifstream cmd_file(command_file.c_str(), std::ios::in);
  SMART_ASSERT(cmd_file.good());

  char line[256];
  std::string xline, tok1, tok2, tok3;
  cmd_file.getline(line, 256);  xline = line;
  while (!cmd_file.eof())
    {
      // Skip blank lines and comment lines.
      if ( count_tokens(xline, " \t") > 0 &&
	   (tok1 = extract_token(xline, " \t"))[0] != '#')
	{
	  to_lower( tok1 );  // Make case insensitive.
	  tok2 = extract_token(xline, " \t");  to_lower(tok2);

	  if ( abbreviation(tok1, "default", 3) &&
	       abbreviation(tok2, "tolerance", 3) )
	    {
	      std::string tok = extract_token(xline, " \n\t=,");  to_lower(tok);
	      if (tok == "") Parse_Die(line);

	      if ( abbreviation(tok, "relative", 3) )
		{
		  default_tol.type = RELATIVE;
		  tok = extract_token( xline, " \n\t=," );
		}
	      else if ( abbreviation(tok, "absolute", 3) )
		{
		  default_tol.type = ABSOLUTE;
		  tok = extract_token( xline, " \n\t=," );
		}
	      else if ( abbreviation(tok, "combine", 3) )
		{
		  default_tol.type = COMBINED;
		  tok = extract_token( xline, " \n\t=," );
		}
	      else if ( abbreviation(tok, "eigen_relative", 7) )
		{
		  default_tol.type = EIGEN_REL;
		  tok = extract_token( xline, " \n\t=," );
		}
	      else if ( abbreviation(tok, "eigen_absolute", 7) )
		{
		  default_tol.type = EIGEN_ABS;
		  tok = extract_token( xline, " \n\t=," );
		}
	      else if ( abbreviation(tok, "eigen_combine", 7) )
		{
		  default_tol.type = EIGEN_COM;
		  tok = extract_token( xline, " \n\t=," );
		}
	      else if ( abbreviation(tok, "ignore", 3) )
		{
		  default_tol.type = IGNORE;
		  tok = extract_token( xline, " \n\t=," );
		}
	      if (tok == "") Parse_Die(line);

	      default_tol.value = To_Double(tok);

	      tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	      if ( abbreviation(tok, "floor", 3) )
		{
		  tok = extract_token( xline, " \n\t=," );
		  if (tok == "") Parse_Die(line);
		  default_tol.floor = To_Double(tok);
		}
	      default_tol_specified = 1;
	    }
	  else if ( abbreviation(tok1, "max", 3) &&
		    abbreviation(tok2, "names", 3) )
	    {
	      std::string tok = extract_token( xline, " \n\t=" );
	      if (tok != "" && tok[0] != '#')
		{
		  errno = 0;
		  int tmp = atoi( tok.c_str() );  SMART_ASSERT(errno == 0);
		  if (tmp > 0) Set_Max_Names(tmp);
		}
	      else
		{
		  std::cout << "exodiff: ERROR:  expected an integer "
		    "after the \"MAX NAMES\" keyword.  "
		    "Aborting..." << std::endl;
		  exit(1);
		}
	    }
	  else if ( abbreviation(tok1, "final", 3) &&
		    abbreviation(tok2, "time", 3) )
	    {
	      tok3 = extract_token(xline, " \t");  to_lower(tok3);
	      if (!abbreviation(tok3, "tolerance", 3)) {
		std::cout << "exodiff: ERROR:  expected \"TOLERANCE\" "
			  << "after the \"FINAL TIME\" keyword. "
			  <<  "Found \"" << tok3 << "\" instead. Aborting..." << std::endl;
		exit(1);
	      }
	      std::string tok = extract_token( xline, " \n\t=," );
	      if (tok == "") Parse_Die(line);
	      final_time_tol.value = To_Double(tok);
	    }
	  else if ( abbreviation(tok1, "return", 3) &&
		    abbreviation(tok2, "status", 3) )
	    {
	      exit_status_switch = true;
	    }
	  else if ( abbreviation(tok1, "ignore", 3) &&
		    abbreviation(tok2, "status", 3) )
	    {
	      exit_status_switch = false;
	    }
	  else if ( abbreviation(tok1, "exclude", 3) &&
		    abbreviation(tok2, "times", 3) )
	    {
	      std::string tok = extract_token( xline, " \n\t=" );
	      if (tok != "" && tok[0] != '#') parseExcludeTimes(tok, exclude_steps);
	    }
	  else if ( abbreviation(tok1, "apply", 3) &&
		    abbreviation(tok2, "matching", 3) )
	    {
	      map_flag = DISTANCE;
	    }
	  else if ( abbreviation(tok1, "calculate", 3) &&
		    abbreviation(tok2, "norms", 3) )
	    {
	      doNorms = true;
	    }
	  else if ( tok1 == "nodeset" &&
		    abbreviation(tok2, "match", 3) )
	    {
	      nsmap_flag = true;
	    }
	  else if ( tok1 == "pedantic")
	    {
	      pedantic = true;
	    }
	  else if ( tok1 == "interpolate")
	    {
	      interpolating = true;
	    }
	  else if ( tok1 == "sideset" &&
		    abbreviation(tok2, "match", 3) )
	    {
	      ssmap_flag = true;
	    }
	  else if ( abbreviation(tok1, "short", 3) &&
		    abbreviation(tok2, "blocks", 3) )
	    {
	      short_block_check = true;
	    }
	  else if ( tok1 == "no" &&
		    abbreviation(tok2, "short", 3) )
	    {
	      short_block_check = false;
	    }
	  else if ( abbreviation(tok1, "ignore", 3) &&
		    abbreviation(tok2, "case", 3) )
	    {
	      nocase_var_names = true;
	    }
	  else if ( abbreviation(tok1, "case", 3) &&
		    abbreviation(tok2, "sensitive", 3) )
	    {
	      nocase_var_names = false;
	    }
	  else if ( abbreviation(tok1, "ignore", 3) &&
		    abbreviation(tok2, "maps", 3) )
	    {
	      ignore_maps = true;
	    }
	  else if ( abbreviation(tok1, "ignore", 3) &&
		    abbreviation(tok2, "nans", 3) )
	    {
	      ignore_nans = true;
	    }
	  else if ( abbreviation(tok1, "ignore", 3) &&
		    abbreviation(tok2, "dups", 3) )
	    {
	      ignore_dups = true;
	    }
	  else if ( abbreviation(tok1, "ignore", 3) &&
		    abbreviation(tok2, "attributes", 3) )
	    {
	      ignore_attributes = true;
	    }
	  else if ( tok1 == "step" && tok2 == "offset" )
	    {
	      std::string tok = extract_token( xline, " \n\t=" );
	      if ( abbreviation(tok, "automatic", 4) ) {
		time_step_offset = -1;
	      } else if ( abbreviation(tok, "match", 4) ) {
		time_step_offset = -2;
	      } else {
		errno = 0;
		time_step_offset = atoi(tok.c_str());
		SMART_ASSERT(errno == 0);
	      }
	    }
	  else if ( abbreviation(tok1, "coordinates", 4) )
	    {
	      if (default_tol_specified) {
		coord_tol = default_tol;
	      } else {
		coord_tol.type  = ABSOLUTE; // These should correspond to
		coord_tol.value = 1.e-6;   // the defaults at the top of
		coord_tol.floor = 0.0;     // this file.
	      }

	      if (tok2 != "" && tok2[0] != '#')
		{
		  // If rel or abs is specified, then the tolerance must
		  // be specified.
		  if ( abbreviation(tok2, "relative", 3) )
		    {
		      coord_tol.type = RELATIVE;
		      tok2 = extract_token( xline, " \n\t=" );
		      if (tok2 == "") Parse_Die(line);
		      coord_tol.value = To_Double(tok2);
		    }
		  else if ( abbreviation(tok2, "absolute", 3) )
		    {
		      coord_tol.type = ABSOLUTE;
		      tok2 = extract_token( xline, " \n\t=" );
		      if (tok2 == "") Parse_Die(line);
		      coord_tol.value = To_Double(tok2);
		    }
		  else if ( abbreviation(tok2, "combine", 3) )
		    {
		      coord_tol.type = COMBINED;
		      tok2 = extract_token( xline, " \n\t=" );
		      if (tok2 == "") Parse_Die(line);
		      coord_tol.value = To_Double(tok2);
		    }
		  else if ( abbreviation(tok2, "eigen_relative", 7) )
		    {
		      coord_tol.type = EIGEN_REL;
		      tok2 = extract_token( xline, " \n\t=" );
		      if (tok2 == "") Parse_Die(line);
		      coord_tol.value = To_Double(tok2);
		    }
		  else if ( abbreviation(tok2, "eigen_absolute", 7) )
		    {
		      coord_tol.type = EIGEN_ABS;
		      tok2 = extract_token( xline, " \n\t=" );
		      if (tok2 == "") Parse_Die(line);
		      coord_tol.value = To_Double(tok2);
		    }
		  else if ( abbreviation(tok2, "eigen_combine", 7) )
		    {
		      coord_tol.type = EIGEN_COM;
		      tok2 = extract_token( xline, " \n\t=" );
		      if (tok2 == "") Parse_Die(line);
		      coord_tol.value = To_Double(tok2);
		    }
		  else if ( abbreviation(tok2, "ignore", 3) )
		    {
		      coord_tol.type = IGNORE;
		      coord_tol.value = 0.0;
		    }
		  else if ( abbreviation(tok2, "floor", 3) )
		    {
		      tok2 = extract_token( xline, " \n\t=" );
		      if (tok2 == "") Parse_Die(line);
		      coord_tol.floor = To_Double(tok2);
		    }

		  tok2 = extract_token( xline, " \n\t=," );  to_lower(tok2);
		  if ( abbreviation(tok2, "floor", 3) )
		    {
		      tok2 = extract_token( xline, " \n\t=," );
		      if (tok2 == "") Parse_Die(line);
		      coord_tol.floor = To_Double(tok2);
		    }
		}
	    }
	  else if (tok1 == "time" && abbreviation(tok2, "steps", 4) )
	    {
	      time_tol = default_tol;

	      std::string tok = extract_token( xline, " \n\t=" );  to_lower(tok);
	      if (tok != "" && tok[0] != '#')
		{
		  // If rel or abs is specified, then the tolerance
		  // must be specified.
		  if ( abbreviation(tok, "relative", 3) )
		    {
		      time_tol.type = RELATIVE;
		      tok = extract_token( xline, " \n\t=" );
		      if (tok == "") Parse_Die(line);
		      time_tol.value = To_Double(tok);
		    }
		  else if ( abbreviation(tok, "absolute", 3) )
		    {
		      time_tol.type = ABSOLUTE;
		      tok = extract_token( xline, " \n\t=" );
		      if (tok == "") Parse_Die(line);
		      time_tol.value = To_Double(tok);
		    }
		  else if ( abbreviation(tok, "combine", 3) )
		    {
		      time_tol.type = COMBINED;
		      tok = extract_token( xline, " \n\t=" );
		      if (tok == "") Parse_Die(line);
		      time_tol.value = To_Double(tok);
		    }
		  else if ( abbreviation(tok, "ignore", 3) )
		    {
		      time_tol.type = IGNORE;
		      time_tol.value = 0.0;
		    }
		  else if ( abbreviation(tok, "floor", 3) )
		    {
		      tok = extract_token( xline, " \n\t=" );
		      if (tok == "") Parse_Die(line);
		      time_tol.floor = To_Double(tok);
		    }

		  tok2 = extract_token( xline, " \n\t=," );  to_lower(tok2);
		  if ( abbreviation(tok2, "floor", 3) )
		    {
		      tok2 = extract_token( xline, " \n\t=," );
		      if (tok2 == "") Parse_Die(line);
		      time_tol.floor = To_Double(tok2);
		    }
		}
	    }
	  else if ( abbreviation(tok1, "global", 4) &&
		    abbreviation(tok2, "variables", 3) )
	    {
	      glob_var_default = default_tol;
	      xline = Parse_Variables(xline, cmd_file,
				      glob_var_do_all_flag,
				      glob_var_default,
				      glob_var_names,
				      glob_var,
				      max_number_of_names);

	      Check_Parsed_Names(glob_var_names, glob_var_do_all_flag);

	      if (!xline.empty()) strncpy(line, xline.c_str(), 255);
	      else                strcpy(line, "");

	      continue;
	    }
	  else if ( abbreviation(tok1, "nodal", 4) &&
		    abbreviation(tok2, "variables", 3) )
	    {
	      node_var_default = default_tol;
	      xline = Parse_Variables(xline, cmd_file,
				      node_var_do_all_flag,
				      node_var_default,
				      node_var_names,
				      node_var,
				      max_number_of_names);

	      Check_Parsed_Names(node_var_names, node_var_do_all_flag);

	      if (!xline.empty()) strncpy(line, xline.c_str(), 255);
	      else                strcpy(line, "");

	      continue;
	    }
	  else if ( abbreviation(tok1, "element", 4) &&
		    abbreviation(tok2, "variables", 3) )
	    {
	      elmt_var_default = default_tol;
	      xline = Parse_Variables(xline, cmd_file,
				      elmt_var_do_all_flag,
				      elmt_var_default,
				      elmt_var_names,
				      elmt_var,
				      max_number_of_names);

	      Check_Parsed_Names(elmt_var_names, elmt_var_do_all_flag);

	      if (!xline.empty()) strncpy(line, xline.c_str(), 255);
	      else                strcpy(line, "");

	      continue;
	    }
	  else if ( tok1 == "nodeset" &&
		    abbreviation(tok2, "variables", 3) )
	    {
	      ns_var_default = default_tol;
	      xline = Parse_Variables(xline, cmd_file,
				      ns_var_do_all_flag,
				      ns_var_default,
				      ns_var_names,
				      ns_var,
				      max_number_of_names);

	      Check_Parsed_Names(ns_var_names, ns_var_do_all_flag);

	      if (!xline.empty()) strncpy(line, xline.c_str(), 255);
	      else                strcpy(line, "");

	      continue;
	    }
	  else if ( abbreviation(tok1, "sideset", 4) &&
		    abbreviation(tok2, "variables", 3) )
	    {
	      ss_var_default = default_tol;
	      xline = Parse_Variables(xline, cmd_file,
				      ss_var_do_all_flag,
				      ss_var_default,
				      ss_var_names,
				      ss_var,
				      max_number_of_names);

	      Check_Parsed_Names(ss_var_names, ss_var_do_all_flag);

	      if (!xline.empty()) strncpy(line, xline.c_str(), 255);
	      else                strcpy(line, "");

	      continue;
	    }
	  else if ( abbreviation(tok1, "element", 4) &&
		    abbreviation(tok2, "attributes", 3) )
	    {
	      elmt_att_default = default_tol;
	      xline = Parse_Variables(xline, cmd_file,
				      elmt_att_do_all_flag,
				      elmt_att_default,
				      elmt_att_names,
				      elmt_att,
				      max_number_of_names);

	      Check_Parsed_Names(elmt_att_names, elmt_att_do_all_flag);

	      if (!xline.empty()) strncpy(line, xline.c_str(), 255);
	      else                strcpy(line, "");

	      continue;
	    }
	  else
	    Parse_Die(line);
	}

      cmd_file.getline(line, 256);  xline = line;
    }
}

namespace {
  std::string Parse_Variables(std::string xline, std::ifstream& cmd_file,
			      bool& all_flag,
			      Tolerance &def_tol,
			      std::vector<std::string>& names,
			      std::vector<Tolerance> &toler,
			      int max_names)
{
  char line[256];

  toler[0] = def_tol;

  std::string tok = extract_token( xline, " \n\t=," );  to_lower(tok);
  if (tok != "")
    {
      if (tok != "(all)" && tok != "all" &&
	  !abbreviation(tok, "relative",    3) &&
	  !abbreviation(tok, "absolute",    3) &&
	  !abbreviation(tok, "combine", 3) &&
	  !abbreviation(tok, "ulps_float", 6) &&
	  !abbreviation(tok, "ulps_double", 6) &&
	  !abbreviation(tok, "eigen_relative",    7) &&
	  !abbreviation(tok, "eigen_absolute",    7) &&
	  !abbreviation(tok, "eigen_combine", 7) &&
	  !abbreviation(tok, "ignore", 3) &&
	  !abbreviation(tok, "floor",       3) )
	{
	  std::cout << "exodiff: error in parsing command file: unrecognized "
		  "keyword \"" << tok << "\"" << std::endl;
	  exit(1);
	}

      if (tok == "(all)" || tok == "all") {
	all_flag = true;
	tok = extract_token( xline, " \n\t=," );
      }

      // If rel or abs is specified, then the tolerance must be specified.
      if ( abbreviation(tok, "relative", 3) )
	{
	  def_tol.type = RELATIVE;
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "floor" || tok == "") {
	    std::cout << "exodiff: Input file specifies a tolerance type "
		    "but no tolerance" << std::endl;
	    exit(1);
	  }
	  def_tol.value = To_Double(tok);
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}
      else if ( abbreviation(tok, "absolute", 3) )
	{
	  def_tol.type = ABSOLUTE;
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "floor" || tok == "") {
	    std::cout << "exodiff: Input file specifies a tolerance type "
		    "but no tolerance" << std::endl;
	    exit(1);
	  }
	  def_tol.value = To_Double(tok);
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}
      else if ( abbreviation(tok, "combine", 3) )
	{
	  def_tol.type = COMBINED;
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "floor" || tok == "") {
	    std::cout << "exodiff: Input file specifies a tolerance type "
		    "but no tolerance" << std::endl;
	    exit(1);
	  }
	  def_tol.value = To_Double(tok);
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}
      else if ( abbreviation(tok, "ulps_float", 6) )
	{
	  def_tol.type = ULPS_FLOAT;
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "floor" || tok == "") {
	    std::cout << "exodiff: Input file specifies a tolerance type "
		    "but no tolerance" << std::endl;
	    exit(1);
	  }
	  def_tol.value = To_Double(tok);
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}
      else if ( abbreviation(tok, "ulps_double", 6) )
	{
	  def_tol.type = ULPS_DOUBLE;
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "floor" || tok == "") {
	    std::cout << "exodiff: Input file specifies a tolerance type "
		    "but no tolerance" << std::endl;
	    exit(1);
	  }
	  def_tol.value = To_Double(tok);
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}
      else if ( abbreviation(tok, "eigen_relative", 7) )
	{
	  def_tol.type = EIGEN_REL;
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "floor" || tok == "") {
	    std::cout << "exodiff: Input file specifies a tolerance type "
		    "but no tolerance" << std::endl;
	    exit(1);
	  }
	  def_tol.value = To_Double(tok);
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}
      else if ( abbreviation(tok, "eigen_absolute", 7) )
	{
	  def_tol.type = EIGEN_ABS;
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "floor" || tok == "") {
	    std::cout << "exodiff: Input file specifies a tolerance type "
		    "but no tolerance" << std::endl;
	    exit(1);
	  }
	  def_tol.value = To_Double(tok);
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}
      else if ( abbreviation(tok, "eigen_combine", 7) )
	{
	  def_tol.type = EIGEN_COM;
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "floor" || tok == "") {
	    std::cout << "exodiff: Input file specifies a tolerance type "
		    "but no tolerance" << std::endl;
	    exit(1);
	  }
	  def_tol.value = To_Double(tok);
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}
      else if ( abbreviation(tok, "ignore", 3) )
	{
	  def_tol.type = IGNORE;
	  def_tol.value = 0.0;
	  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
	}


      if ( abbreviation(tok, "floor", 3) )
	{
	  tok = extract_token( xline, " \n\t=," );
	  if (tok == "" || tok[0] == '#') {
	    std::cout << "exodiff: Floor specified but couldn't find value"
		 << std::endl;
	    exit(1);
	  }
	  def_tol.floor = To_Double(tok);
	}
    }

  for (int i = 0; i < max_names; ++i) {
    toler[i] = def_tol;
  }

  cmd_file.getline(line, 256);  xline = line;
  while (!cmd_file.eof())
    {
      if (xline.empty() || (xline[0] != '\t' && first_character(xline) != '#'))
	break;

      if ( first_character(xline) != '#' )
	{
	  tok = extract_token(xline);
	  chop_whitespace( tok );
	  if (tok == "") continue;  // Found tab but no name given.

	  int idx = names.size();
	  if (idx >= max_names) {
	    std::cout << "exodiff: Number of names in tabbed list is larger "
		    "than current limit of " << max_names
		 << ".  To increase, use \"-maxnames <int>\" on the "
		    "command line or \"MAX NAMES <int>\" in the command "
		    "file.  Aborting..." << std::endl;
	    exit(1);
	  }

	  if (tok[0] == '!')
	    {
	      // A "!" in front of a name means to exclude the name so no
	      // need to look for difference type and tolerance.
	      std::string tmp = tok;
	      if (extract_token(tmp,"!") != "") names.push_back(tok);
	      cmd_file.getline(line, 256);  xline = line;
	      continue;
	    }
	  names.push_back(tok);

	  tok = extract_token(xline);  to_lower(tok);

	  if (tok != "" && tok[0] != '#')
	    {
	      if ( abbreviation(tok, "relative", 3) )
		{
		  toler[idx].type = RELATIVE;
		  tok = extract_token(xline," \n\t=,");
		}
	      else if ( abbreviation(tok, "absolute", 3) )
		{
		  toler[idx].type = ABSOLUTE;
		  tok = extract_token(xline," \n\t=,");
		}
	      else if ( abbreviation(tok, "combine", 3) )
		{
		  toler[idx].type = COMBINED;
		  tok = extract_token(xline," \n\t=,");
		}
	      else if ( abbreviation(tok, "eigen_relative", 7) )
		{
		  toler[idx].type = EIGEN_REL;
		  tok = extract_token(xline," \n\t=,");
		}
	      else if ( abbreviation(tok, "eigen_absolute", 7) )
		{
		  toler[idx].type = EIGEN_ABS;
		  tok = extract_token(xline," \n\t=,");
		}

	      else if ( abbreviation(tok, "eigen_com", 7) )
		{
		  toler[idx].type = EIGEN_COM;
		  tok = extract_token(xline," \n\t=,");
		}

	      if ( abbreviation(tok, "floor", 3) )
		{
		  toler[idx].value = def_tol.value;

		  tok = extract_token(xline," \n\t=,");
		  if (tok == "") Parse_Die(line);
		  toler[idx].floor = To_Double(tok);
		}
	      else
		{
		  if (tok == "") Parse_Die(line);
		  toler[idx].value = To_Double(tok);

		  tok = extract_token(xline," \n\t=,");  to_lower(tok);
		  if ( abbreviation(tok, "floor", 3) )
		    {
		      tok = extract_token(xline," \n\t=,");
		      if (tok == "") Parse_Die(line);
		      toler[idx].floor = To_Double(tok);
		    }
		  else
		    toler[idx].floor = def_tol.floor;
		}
	    }
	  else
	    {
	      toler[idx] = def_tol;
	    }
	}

      cmd_file.getline(line, 256);  xline = line;
    }

  if (names.size() == 0) all_flag = true;

  return xline;
}

  void tolerance_help()
  {
    std::cout
      << "\n Tolerance Help:\n"
      << "\n"
      << "\t Relative difference  |val1 - val2| / max(|val1|, |val2|)\n"
      << "\t Absolute difference  |val1 - val2|\n"
      << "\t Combined difference  |val1 - val2| / max(tol, tol * max(|val1|, |val2|))\n"
      << "\t Eigen_relative difference  ||val1| - |val2|| / max(|val1|,|val2|)\n"
      << "\t Eigen_absolute difference  ||val1| - |val2||\n"
      << "\t Eigen_combined difference  ||val1| - |val2|| / max(tol, tol * max(|val1|, |val2|))\n"
      << "\t Ulps_float difference  -- Calculate number of representable floats between the two values\n"
      << "\t Ulps_double difference  -- Calculate number of representable doubles between the two values\n"
      << "\n"
      << "\t Values are considered equal if |val1| <= floor and |val2| <= floor;\n"
      << "\t where floor is a user-specified value (-Floor option). Otherwise the difference is\n"
      << "\t computed using one of the above formulas and compared to a tolerance.\n"
      << "\t If the difference is greater than the tolerance, then the databases\n"
      << "\t are different.  At the end of execution, a summary of the differences\n"
      << "\t found is output.\n"
      << "\t \n"
      << "\t By default:\n"
      << "\t * All results variables and attributes are compared using a relative difference\n"
      << "\t   of 10^{-6} (about 6 significant digits) and a floor of 0.0.\n"
      << "\t * Nodal locations are compared using {absolute difference} with\n"
      << "\t   a tolerance of 10^{-6} and a floor of 0.0.\n"
      << "\t * Time step values are compared using relative difference tolerance of 10^{-6}\n"
      << "\t   and a floor of 10^{-15}.\n"
      << "\n\n";
  }

  void file_help()
  {
    std::cout
      << "\n  Command file syntax:\n"
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
      << "         - The interpolation option, \"-interpolate\", can be turned\n"
      << "           on with the INTERPOLATE keyword.\n"
      << "         - The final time tolerance, \"-final_time_tolerance <tol>\", can be turned\n"
      << "           on with the FINAL TIME TOLERANCE keyword.\n"
      << "         - The calculation of the L2 norm of differences \"-norms\", can be turned\n"
      << "           on with the CALCULATE NORMS keyword.\n"
      << "         - The exit status return option, \"-stat\", can be turned on with the \n"
      << "           RETURN STATUS keyword.\n"
      << "         - The ignore exit status return option, \"-ignore_status\", can be turned on with the \n"
      << "           IGNORE STATUS keyword.\n"
      << "         - The pedantic compare option, \"-pedantic\", can be turned on with the \n"
      << "           PEDANTIC keyword.\n"
      << std::endl;
  }
}
