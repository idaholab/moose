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

#include "parsing.h"
#include "smart_assert.h"
#include "Specifications.h"
#include "exoII_read.h"
#include "exodiff.h"
#include "stringx.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <errno.h>

using namespace std;

extern void Print_Banner(const char *prefix);

namespace {
  void show_copyright();
}

int File_Exists(const string & fname)
{
  if (fname.empty()) return 0;
  ifstream file_check(fname.c_str(), ios::in);
  if (file_check.fail()) return 0;
  file_check.close();
  return 1;
}

void Parse_Steps_Option(const string &option)
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

  const char *tokens = option.c_str();
  if (tokens != NULL) {
    if (strchr(tokens, ':') != NULL) {
      // The string contains a separator

      int vals[3];
      vals[0] = specs.time_step_start;
      vals[1] = specs.time_step_stop;
      vals[2] = specs.time_step_increment;

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
      specs.time_step_start     = vals[0];
      specs.time_step_stop      = vals[1];
      specs.time_step_increment = vals[2];
    } else {
      // Does not contain a separator, min == max
      specs.time_step_start = specs.time_step_stop = strtol(tokens, NULL, 0);
    }
  }
}

double To_Double(const string & str_val)
{
  SMART_ASSERT(str_val.size() > 0);
  
  char* endptr; errno = 0;
  double val = strtod(str_val.c_str(), &endptr);
  
  if (errno == ERANGE) {
    cout << "exodiff: ERROR:  Overflow or underflow occured when trying"
         << " to parse command line tolerance.  Aborting..." << endl;
    exit(1);
  }
  errno = 0;
  
  if (val < 0.0) {
    cout << "exodiff: ERROR:  Parsed a negative value \""
         << val << "\".  Aborting..." << endl;
    exit(1);
  }
  
  return val;
}


void Check_Parsed_Names(vector<string>& names, bool& all_flag)
{
  int num_include = 0;
  int num_exclude = 0;
  for (unsigned i = 0; i < names.size(); ++i)
  {
    SMART_ASSERT(names[i] != "");
    if (names[i][0] == '!')
      ++num_exclude;
    else
      ++num_include;
  }
  if (!all_flag && num_include > 0 && num_exclude > 0)
  {
    cout << "exodiff: ERROR: Parsing error: Cannot specify both "
            "variables to include and exclude without using the "
            "'(all)' specifier.  Aborting..." << endl;
    exit(1);
  }
  if (num_include == 0 && num_exclude > 0) all_flag = true;
}


void parseExcludeTimes(string exclude_arg)
{
  string arg_copy = exclude_arg;
  
  int num_excluded_steps = 0;
  
  // first pass just counts the number of excluded time steps:
  
  string tok = extract_token( exclude_arg, "," );
  while (tok.size() > 0)
  {
    string subtok = extract_token( tok, "-" );
    SMART_ASSERT(subtok.size() > 0);
    
    errno = 0;
    int ival1 = atoi( subtok.c_str() );  SMART_ASSERT(errno == 0);
    
    if (ival1 < 1) {
      cout << "exodiff: Error parsing exclusion times from command "
              "line .. value was less than 1" << endl;
      exit(1);
    }
    
    ++num_excluded_steps;
    
    subtok = extract_token( tok, "-" );
    if (subtok.size() > 0)
    {
      errno = 0;
      int ival2 = atoi( subtok.c_str() );  SMART_ASSERT(errno == 0);
      
      if (ival2 < 1) {
        cout << "exodiff: Error parsing exclusion times from command "
                "line .. value was less than 1" << endl;
        exit(1);
      }
      
      if (ival1 < ival2) {
        for (int i = ival1+1; i <= ival2; ++i) ++num_excluded_steps;
      }
      else if (ival1 > ival2) {
        cout << "exodiff: Error parsing exclusion times from command "
                "line .. first value in a range was greater than the "
                "second" << endl;
        exit(1);
      }
    }
    
    tok = extract_token( exclude_arg, "," );
  }
  
  if (num_excluded_steps > 0)
  {
    specs.allocateExcludeSteps( num_excluded_steps );
    
    // second pass collects the excluded time steps
    
    exclude_arg = arg_copy;
    num_excluded_steps = 0;
    
    tok = extract_token( exclude_arg, "," );
    while (tok.size() > 0)
    {
      string subtok = extract_token( tok, "-" );
      SMART_ASSERT(subtok.size() > 0);
      
      errno = 0;
      int ival1 = atoi( subtok.c_str() );  SMART_ASSERT(errno == 0);
      
      specs.exclude_steps[num_excluded_steps++] = ival1;
      
      subtok = extract_token( tok, "-" );
      if (subtok.size() > 0)
      {
        errno = 0;
        int ival2 = atoi( subtok.c_str() );  SMART_ASSERT(errno == 0);
        
        for (int i = ival1+1; i <= ival2; ++i)
          specs.exclude_steps[num_excluded_steps++] = i;
      }
      
      tok = extract_token( exclude_arg, "," );
    }
  }
  
//   cout << "Excluded steps:";
//   for (int j = 0; j < num_excluded_steps; ++j)
//     cout << " " << specs.exclude_steps[j];
//   cout << endl;
}

bool Parse_Token(string& tok, string& opt_arg)
{
  bool used_opt_arg = false;
  
  if ( tok.empty() ) {
  }
  else if ( tok == "-h" || tok == "-help" ) {
    Echo_Usage();
    exit(0);
  }
  else if ( tok == "-H" || tok == "-Help" || tok == "-man" ) {
    Echo_Help(opt_arg);
    exit(0);
  }
  else if ( tok == "-copyright" ) {
    Print_Banner(" ");
    show_copyright();
  }
  else if ( abbreviation(tok, "-summary", 4) ) {
    specs.summary_flag = true;
    if ( abbreviation(opt_arg, "no_coord_sep", 4) ) {
      specs.coord_sep = false;
      used_opt_arg = true;
    }
  }
  else if ( tok == "-x" || abbreviation(tok, "-exclude", 3) ) {
    parseExcludeTimes(opt_arg);
    used_opt_arg = true;
  }
  else if ( abbreviation(tok, "-version", 2) ) {
    cout << Version() << endl;
    exit(0);
  }
  else if ( abbreviation(tok, "-tolerance", 2) ) {
    specs.default_tol.value = To_Double(opt_arg);
    used_opt_arg = true;
  }
  else if ( abbreviation(tok, "-Floor", 2) ) {
    specs.default_tol.floor = To_Double(opt_arg);
    used_opt_arg = true;
  }
  else if ( abbreviation(tok, "-TimeStepOffset", 2) ) {
    errno = 0;
    specs.time_step_offset =  atoi(opt_arg.c_str());  SMART_ASSERT(errno == 0);
    used_opt_arg = true;
  }
  else if (tok == "-TA") {
    specs.time_step_offset =  -1; // Signifies automatic offset calculation.
  }
  else if (tok == "-TM") {
    specs.time_step_offset =  -2; // Signifies automatic offset calculation -- closest match
  }
  else if ( tok == "-steps") {
    Parse_Steps_Option(opt_arg);
    used_opt_arg = true;
  }
  else if ( abbreviation(tok, "-quiet", 2) ) {
    specs.quiet_flag = true;
  }
  else if ( abbreviation(tok, "-show_all_diffs", 8) ) {
    specs.show_all_diffs = true;
  }
  else if ( abbreviation(tok, "-partial", 2) ) {
    specs.map_flag = PARTIAL;
  }
  else if ( abbreviation(tok, "-match_ids", 9) ) {
    specs.map_flag = USE_FILE_IDS;
  }
  else if ( abbreviation(tok, "-match_file_order", 9) ) {
    specs.map_flag = FILE_ORDER;
  }
  else if ( abbreviation(tok, "-map", 2) ) {
    specs.map_flag = DISTANCE;
  }
  else if ( tok == "-nsmap" ) {
    specs.nsmap_flag = true;
  }
  else if ( tok == "-no_nsmap" ) {
    specs.nsmap_flag = false;
  }
  else if ( tok == "-ssmap" ) {
    specs.ssmap_flag = true;
  }
  else if ( tok == "-no_ssmap" ) {
    specs.ssmap_flag = false;
  }
  else if ( abbreviation(tok, "-short", 2) ) {
    specs.short_block_check = true;
  }
  else if ( tok == "-no_short") {
    specs.short_block_check = false;
  }
  else if ( tok == "-nosymm" ) {
    specs.noSymmetricNameCheck = true;
  }
  else if ( tok == "-norms" ) {
    specs.doNorms = true;
  }
  else if (abbreviation(tok, "-allow_name_mismatch", 8)) {
    specs.allowNameMismatch = true;
  }
  else if ( tok == "-i" || tok == "-ignore_case" ) {
    specs.nocase_var_names = true;
  }
  else if ( tok == "-case_sensitive" ) {
    specs.nocase_var_names = false;
  }
  else if ( tok == "-ignore_maps" ) {
    specs.ignore_maps = true;
  }
  else if ( tok == "-ignore_nans" ) {
    specs.ignore_nans = true;
  }
  else if ( tok == "-ignore_dups" ) {
    specs.ignore_dups = true;
  }
  else if ( abbreviation(tok, "-ignore_attributes", 12) ) {
    specs.ignore_attributes = true;
  }
  else if ( abbreviation(tok, "-relative", 2) ) {
    specs.output_type      = RELATIVE;  // Change type to relative.
    specs.default_tol.type = RELATIVE;
  }
  else if ( abbreviation(tok, "-absolute", 2) ) {
    specs.output_type      = ABSOLUTE;  // Change type to absolute
    specs.default_tol.type = ABSOLUTE;
  }
  else if ( abbreviation(tok, "-combine", 3) ) {
    specs.output_type      = COMBINED;  // Change type to combine
    specs.default_tol.type = COMBINED;
  }
  else if ( abbreviation(tok, "-eigen_relative", 7) ) {
    specs.output_type      = EIGEN_REL;  // Change type to relative.
    specs.default_tol.type = EIGEN_REL;
  }
  else if ( abbreviation(tok, "-eigen_absolute", 7) ) {
    specs.output_type      = EIGEN_ABS;  // Change type to absolute
    specs.default_tol.type = EIGEN_ABS;
  }
  else if ( abbreviation(tok, "-eigen_combine", 7) ) {
    specs.output_type      = EIGEN_COM;  // Change type to combine
    specs.default_tol.type = EIGEN_COM;
  }
  else if ( tok == "-dumpmap" ) {
    specs.dump_mapping = true;
  }
  else if ( tok == "-show_unmatched" ) {
    specs.show_unmatched = true;
  }
  else if ( tok == "-maxnames" ) {
    errno = 0;
    int tmp = atoi(opt_arg.c_str());  SMART_ASSERT(errno == 0);
    if (tmp > 0) specs.Set_Max_Names(tmp);
    used_opt_arg = true;
  }
  else if ( abbreviation(tok, "-status", 3) ) {
    specs.exit_status_switch = true;
  }
  else if ( tok == "-use_old_floor" ) {
    Tolerance::use_old_floor = true;  // Change type to relative.
  }
  else if ( abbreviation(tok, "-file", 2) ) {
    specs.command_file_name = opt_arg;
    if (!specs.summary_flag && !File_Exists(specs.command_file_name)) {
      cout << "exodiff: Can't open file \"" << specs.command_file_name << "\"." << endl;
      exit(1);
    }
    used_opt_arg = true;
  }
  else if ( tok[0] == '-' ) {
    cout << "exodiff: ERROR:  Unknown option \"" << tok
         << "\".  Aborting..." << endl;
    exit(1);
  }
  return used_opt_arg;
}


void Parse_Die(const char* line)
{
  string sline = line;
  chop_whitespace(sline);
  cout << "exodiff: Error parsing input file, currently at \""
       << sline << "\"." << endl;
  exit(1);
}

string Parse_Variables(string xline, ifstream& cmd_file,
                        bool& all_flag,
                        Tolerance &def_tol,
                        vector<string>*& names_ptr,
                        Tolerance toler[])
{
  char line[256];
  
  def_tol = specs.default_tol;
  toler[0] = def_tol;
  
  string tok = extract_token( xline, " \n\t=," );  to_lower(tok);
  if (tok != "")
    {
      if (tok != "(all)" && tok != "all" &&
          !abbreviation(tok, "relative",    3) &&
          !abbreviation(tok, "absolute",    3) &&
          !abbreviation(tok, "combine", 3) &&
          !abbreviation(tok, "eigen_relative",    7) &&
          !abbreviation(tok, "eigen_absolute",    7) &&
          !abbreviation(tok, "eigen_combine", 7) &&
          !abbreviation(tok, "floor",       3) )
        {
          cout << "exodiff: error in parsing command file: unrecognized "
                  "keyword \"" << tok << "\"" << endl;
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
            cout << "exodiff: Input file specifies a tolerance type "
                    "but no tolerance" << endl;
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
            cout << "exodiff: Input file specifies a tolerance type "
                    "but no tolerance" << endl;
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
            cout << "exodiff: Input file specifies a tolerance type "
                    "but no tolerance" << endl;
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
            cout << "exodiff: Input file specifies a tolerance type "
                    "but no tolerance" << endl;
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
            cout << "exodiff: Input file specifies a tolerance type "
                    "but no tolerance" << endl;
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
            cout << "exodiff: Input file specifies a tolerance type "
                    "but no tolerance" << endl;
            exit(1);
          }
          def_tol.value = To_Double(tok);
          tok = extract_token( xline, " \n\t=," );  to_lower(tok);
        }
        
        
      if ( abbreviation(tok, "floor", 3) )
        {
          tok = extract_token( xline, " \n\t=," );
          if (tok == "" || tok[0] == '#') {
            cout << "exodiff: Floor specified but couldn't find value"
                 << endl;
            exit(1);
          }
          def_tol.floor = To_Double(tok);
        }
    }
  
  for (int i = 0; i < specs.max_number_of_names; ++i) {
    toler[i] = def_tol;
  }
  
  SMART_ASSERT(names_ptr != 0);
  vector<string>& names = *names_ptr;  // Make an alias.
  
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
          if (idx >= specs.max_number_of_names) {
            cout << "exodiff: Number of names in tabbed list is larger "
                    "than current limit of " << specs.max_number_of_names
                 << ".  To increase, use \"-maxnames <int>\" on the "
                    "command line or \"MAX NAMES <int>\" in the command "
                    "file.  Aborting..." << endl;
            exit(1);
          }
      
          if (tok[0] == '!')
            {
              // A "!" in front of a name means to exclude the name so no
              // need to look for difference type and tolerance.
              string tmp = tok;
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

void Parse_Command_Line(int argc, char* argv[],
                        string& file1_name,
                        string& file2_name,
                        string& diffile_name)
{
  // This is a temporary kluge to enable sierra users to specify the
  // -stat flag while running regression tests.
  char *stat_flag = (char*)getenv("EXODIFF_USE_STAT");
  if (stat_flag != NULL) {
    specs.exit_status_switch = 1;
    cout << "exodiff: Enabling -stat option via environment "
            "variable 'EXODIFF_USE_STAT'.\n";
  }
  specs.quiet_flag = 0;
  // See if the environment variable EXODIFF_OPTIONS is set and parse it...
  // Options are space/tab delimited...
  char *options = (char*)getenv("EXODIFF_OPTIONS");
  if (options != NULL) {
    const char *delimiter = " \t";
    string env_opts(options);
    int arg_count = count_tokens( env_opts, delimiter );
    int i = 0;
    while (i < arg_count) {
      string tok = extract_token( env_opts, delimiter );
      ++i;
      string pos_arg = "";
      if ( i < arg_count && !env_opts.empty() && (env_opts[0] != '-' || isdigit(env_opts[1]))) {
        pos_arg = extract_token( env_opts, delimiter );
        ++i;
      }
      Parse_Token(tok, pos_arg);
    }
  }

  int i = 1;
  while (i < argc) {
    string tok = argv[i];
    if ( !tok.empty() && tok[0] == '-' ) {
      i++;
      string pos_arg = "";
      if (i < argc) { 
        pos_arg = argv[i];
        if ( !pos_arg.empty() && (pos_arg[0] == '-' && !isdigit(pos_arg[1]))) {
          pos_arg = "";
        }
      }
      bool used_argument = Parse_Token(tok, pos_arg);
      if (used_argument)
        i++;
    }
    else if (file1_name == "") {
      file1_name = argv[i++];
    }
    else if (file2_name == "") {
      file2_name = argv[i++];
    }
    else if (diffile_name == "") {
      diffile_name = argv[i++];
    }
  }
  
  if ( (file1_name == "" || file2_name == "") && !specs.summary_flag) {
    Echo_Usage();
    exit(1);
  }
  
  // Reset default tolerances in case the -t flag was given.
  specs.time_tol         = specs.default_tol;
  specs.glob_var_default = specs.default_tol;
  specs.node_var_default = specs.default_tol;
  specs.elmt_var_default = specs.default_tol;
  specs.elmt_att_default = specs.default_tol;
  specs.ns_var_default   = specs.default_tol;
  specs.ss_var_default   = specs.default_tol;

  for (int k = 0; k < specs.max_number_of_names; ++k)
    {
      specs.glob_var[k] = specs.default_tol;
      specs.node_var[k] = specs.default_tol;
      specs.elmt_var[k] = specs.default_tol;
      specs.elmt_att[k] = specs.default_tol;
      specs.ss_var[k]   = specs.default_tol;
      specs.ns_var[k]   = specs.default_tol;
    }
  
  specs.allocateNames();
  
  // Parse command file if it exists.
  if (specs.command_file_name != "")
    {
      int default_tol_specified = 0;
    
      // Set all types to inactive (ignore) by default.
      specs.coord_tol.type = IGNORE;
      specs.time_tol.type  = IGNORE;
    
      ifstream cmd_file(specs.command_file_name.c_str(), ios::in);
      SMART_ASSERT(cmd_file.good());
    
      char line[256];
      string xline, tok1, tok2;
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
                  string tok = extract_token(xline, " \n\t=,");  to_lower(tok);
                  if (tok == "") Parse_Die(line);
          
                  if ( abbreviation(tok, "relative", 3) )
                    {
                      specs.default_tol.type = RELATIVE;
                      tok = extract_token( xline, " \n\t=," );
                    }
                  else if ( abbreviation(tok, "absolute", 3) )
                    {
                      specs.default_tol.type = ABSOLUTE;
                      tok = extract_token( xline, " \n\t=," );
                    }
                  else if ( abbreviation(tok, "combine", 3) )
                    {
                      specs.default_tol.type = COMBINED;
                      tok = extract_token( xline, " \n\t=," );
                    }
                  else if ( abbreviation(tok, "eigen_relative", 7) )
                    {
                      specs.default_tol.type = EIGEN_REL;
                      tok = extract_token( xline, " \n\t=," );
                    }
                  else if ( abbreviation(tok, "eigen_absolute", 7) )
                    {
                      specs.default_tol.type = EIGEN_ABS;
                      tok = extract_token( xline, " \n\t=," );
                    }
                  else if ( abbreviation(tok, "eigen_combine", 7) )
                    {
                      specs.default_tol.type = EIGEN_COM;
                      tok = extract_token( xline, " \n\t=," );
                    }
                  if (tok == "") Parse_Die(line);
          
                  specs.default_tol.value = To_Double(tok);
    
                  tok = extract_token( xline, " \n\t=," );  to_lower(tok);
                  if ( abbreviation(tok, "floor", 3) )
                    {
                      tok = extract_token( xline, " \n\t=," );
                      if (tok == "") Parse_Die(line);
                      specs.default_tol.floor = To_Double(tok);
                    }
                  default_tol_specified = 1;
                }
              else if ( abbreviation(tok1, "max", 3) &&
                        abbreviation(tok2, "names", 3) )
                {
                  string tok = extract_token( xline, " \n\t=" );
                  if (tok != "" && tok[0] != '#')
                    {
                      errno = 0;
                      int tmp = atoi( tok.c_str() );  SMART_ASSERT(errno == 0);
                      if (tmp > 0) specs.Set_Max_Names(tmp);
                    }
                  else
                    {
                      cout << "exodiff: ERROR:  expected an integer "
                              "after the \"MAX NAMES\" keyword.  "
                              "Aborting..." << endl;
                      exit(1);
                    }
                }
              else if ( abbreviation(tok1, "return", 3) &&
                        abbreviation(tok2, "status", 3) )
                {
                  specs.exit_status_switch = true;
                }
              else if ( abbreviation(tok1, "exclude", 3) &&
                        abbreviation(tok2, "times", 3) )
                {
                  string tok = extract_token( xline, " \n\t=" );
                  if (tok != "" && tok[0] != '#') parseExcludeTimes(tok);
                }
              else if ( abbreviation(tok1, "apply", 3) &&
                        abbreviation(tok2, "matching", 3) )
                {
                  specs.map_flag = DISTANCE;
                }
              else if ( abbreviation(tok1, "calculate", 3) &&
                        abbreviation(tok2, "norms", 3) )
                {
                  specs.doNorms = true;
                }
              else if ( tok1 == "nodeset" &&
                        abbreviation(tok2, "match", 3) )
                {
                  specs.nsmap_flag = true;
                }
              else if ( tok1 == "sideset" &&
                        abbreviation(tok2, "match", 3) )
                {
                  specs.ssmap_flag = true;
                }
              else if ( abbreviation(tok1, "short", 3) &&
                        abbreviation(tok2, "blocks", 3) )
                {
                  specs.short_block_check = true;
                }
              else if ( tok1 == "no" &&
                        abbreviation(tok2, "short", 3) )
                {
                  specs.short_block_check = false;
                }
              else if ( abbreviation(tok1, "ignore", 3) &&
                        abbreviation(tok2, "case", 3) )
                {
                  specs.nocase_var_names = true;
                }
              else if ( abbreviation(tok1, "case", 3) &&
                        abbreviation(tok2, "sensitive", 3) )
                {
                  specs.nocase_var_names = false;
                }
              else if ( abbreviation(tok1, "ignore", 3) &&
                        abbreviation(tok2, "maps", 3) )
                {
                  specs.ignore_maps = true;
                }
              else if ( abbreviation(tok1, "ignore", 3) &&
                        abbreviation(tok2, "nans", 3) )
                {
                  specs.ignore_nans = true;
                }
              else if ( abbreviation(tok1, "ignore", 3) &&
                        abbreviation(tok2, "dups", 3) )
                {
                  specs.ignore_dups = true;
                }
              else if ( abbreviation(tok1, "ignore", 3) &&
                        abbreviation(tok2, "attributes", 3) )
                {
                  specs.ignore_attributes = true;
                }
              else if ( tok1 == "step" && tok2 == "offset" )
                {
                  string tok = extract_token( xline, " \n\t=" );
                  if ( abbreviation(tok, "automatic", 4) ) {
                    specs.time_step_offset = -1;
                  } else if ( abbreviation(tok, "match", 4) ) {
                    specs.time_step_offset = -2;
                  } else {
                    errno = 0;
                    specs.time_step_offset = atoi(tok.c_str());
                    SMART_ASSERT(errno == 0);
                  }
                }
              else if ( abbreviation(tok1, "coordinates", 4) )
                {
                  if (default_tol_specified) {
                    specs.coord_tol = specs.default_tol;
                  } else {
                    specs.coord_tol.type  = ABSOLUTE; // These should correspond to
                    specs.coord_tol.value = 1.e-6;   // the defaults at the top of
                    specs.coord_tol.floor = 0.0;     // this file.
                  }
          
                  if (tok2 != "" && tok2[0] != '#')
                    {
                      // If rel or abs is specified, then the tolerance must
                      // be specified.
                      if ( abbreviation(tok2, "relative", 3) )
                        {
                          specs.coord_tol.type = RELATIVE;
                          tok2 = extract_token( xline, " \n\t=" );
                          if (tok2 == "") Parse_Die(line);
                          specs.coord_tol.value = To_Double(tok2);
                        }
                      else if ( abbreviation(tok2, "absolute", 3) )
                        {
                          specs.coord_tol.type = ABSOLUTE;
                          tok2 = extract_token( xline, " \n\t=" );
                          if (tok2 == "") Parse_Die(line);
                          specs.coord_tol.value = To_Double(tok2);
                        }
                      else if ( abbreviation(tok2, "combine", 3) )
                        {
                          specs.coord_tol.type = COMBINED;
                          tok2 = extract_token( xline, " \n\t=" );
                          if (tok2 == "") Parse_Die(line);
                          specs.coord_tol.value = To_Double(tok2);
                        }
                      else if ( abbreviation(tok2, "eigen_relative", 7) )
                        {
                          specs.coord_tol.type = EIGEN_REL;
                          tok2 = extract_token( xline, " \n\t=" );
                          if (tok2 == "") Parse_Die(line);
                          specs.coord_tol.value = To_Double(tok2);
                        }
                      else if ( abbreviation(tok2, "eigen_absolute", 7) )
                        {
                          specs.coord_tol.type = EIGEN_ABS;
                          tok2 = extract_token( xline, " \n\t=" );
                          if (tok2 == "") Parse_Die(line);
                          specs.coord_tol.value = To_Double(tok2);
                        }
                      else if ( abbreviation(tok2, "eigen_combine", 7) )
                        {
                          specs.coord_tol.type = EIGEN_COM;
                          tok2 = extract_token( xline, " \n\t=" );
                          if (tok2 == "") Parse_Die(line);
                          specs.coord_tol.value = To_Double(tok2);
                        }
                      else if ( abbreviation(tok2, "floor", 3) )
                        {
                          tok2 = extract_token( xline, " \n\t=" );
                          if (tok2 == "") Parse_Die(line);
                          specs.coord_tol.floor = To_Double(tok2);
                        }
            
                      tok2 = extract_token( xline, " \n\t=," );  to_lower(tok2);
                      if ( abbreviation(tok2, "floor", 3) )
                        {
                          tok2 = extract_token( xline, " \n\t=," );
                          if (tok2 == "") Parse_Die(line);
                          specs.coord_tol.floor = To_Double(tok2);
                        }
                    }
                }
              else if (tok1 == "time" && abbreviation(tok2, "steps", 4) )
                {
                  specs.time_tol = specs.default_tol;
          
                  string tok = extract_token( xline, " \n\t=" );  to_lower(tok);
                  if (tok != "" && tok[0] != '#')
                    {
                      // If rel or abs is specified, then the tolerance
                      // must be specified.
                      if ( abbreviation(tok, "relative", 3) )
                        {
                          specs.time_tol.type = RELATIVE;
                          tok = extract_token( xline, " \n\t=" );
                          if (tok == "") Parse_Die(line);
                          specs.time_tol.value = To_Double(tok);
                        }
                      else if ( abbreviation(tok, "absolute", 3) )
                        {
                          specs.time_tol.type = ABSOLUTE;
                          tok = extract_token( xline, " \n\t=" );
                          if (tok == "") Parse_Die(line);
                          specs.time_tol.value = To_Double(tok);
                        }
                      else if ( abbreviation(tok, "combine", 3) )
                        {
                          specs.time_tol.type = COMBINED;
                          tok = extract_token( xline, " \n\t=" );
                          if (tok == "") Parse_Die(line);
                          specs.time_tol.value = To_Double(tok);
                        }
                      else if ( abbreviation(tok, "floor", 3) )
                        {
                          tok = extract_token( xline, " \n\t=" );
                          if (tok == "") Parse_Die(line);
                          specs.time_tol.floor = To_Double(tok);
                        }
            
                      tok2 = extract_token( xline, " \n\t=," );  to_lower(tok2);
                      if ( abbreviation(tok2, "floor", 3) )
                        {
                          tok2 = extract_token( xline, " \n\t=," );
                          if (tok2 == "") Parse_Die(line);
                          specs.time_tol.floor = To_Double(tok2);
                        }
                    }
                }
              else if ( abbreviation(tok1, "global", 4) &&
                        abbreviation(tok2, "variables", 3) )
                {
                  xline = Parse_Variables(xline, cmd_file,
                                          specs.glob_var_do_all_flag,
                                          specs.glob_var_default,
                                          specs.glob_var_names,
                                          specs.glob_var);
          
                  Check_Parsed_Names(*specs.glob_var_names, specs.glob_var_do_all_flag);
          
                  if (!xline.empty()) strncpy(line, xline.c_str(), 255);
                  else                strcpy(line, "");
          
                  continue;
                }
              else if ( abbreviation(tok1, "nodal", 4) &&
                        abbreviation(tok2, "variables", 3) )
                {
                  xline = Parse_Variables(xline, cmd_file,
                                          specs.node_var_do_all_flag,
                                          specs.node_var_default,
                                          specs.node_var_names,
                                          specs.node_var);
          
                  Check_Parsed_Names(*specs.node_var_names, specs.node_var_do_all_flag);
          
                  if (!xline.empty()) strncpy(line, xline.c_str(), 255);
                  else                strcpy(line, "");
          
                  continue;
                }
              else if ( abbreviation(tok1, "element", 4) &&
                        abbreviation(tok2, "variables", 3) )
                {
                  xline = Parse_Variables(xline, cmd_file,
                                          specs.elmt_var_do_all_flag,
                                          specs.elmt_var_default,
                                          specs.elmt_var_names,
                                          specs.elmt_var);
          
                  Check_Parsed_Names(*specs.elmt_var_names, specs.elmt_var_do_all_flag);
          
                  if (!xline.empty()) strncpy(line, xline.c_str(), 255);
                  else                strcpy(line, "");
          
                  continue;
                }
              else if ( tok1 == "nodeset" &&
                        abbreviation(tok2, "variables", 3) )
                {
                  xline = Parse_Variables(xline, cmd_file,
                                          specs.ns_var_do_all_flag,
                                          specs.ns_var_default,
                                          specs.ns_var_names,
                                          specs.ns_var);
          
                  Check_Parsed_Names(*specs.ns_var_names, specs.ns_var_do_all_flag);
          
                  if (!xline.empty()) strncpy(line, xline.c_str(), 255);
                  else                strcpy(line, "");
          
                  continue;
                }
              else if ( abbreviation(tok1, "sideset", 4) &&
                        abbreviation(tok2, "variables", 3) )
                {
                  xline = Parse_Variables(xline, cmd_file,
                                          specs.ss_var_do_all_flag,
                                          specs.ss_var_default,
                                          specs.ss_var_names,
                                          specs.ss_var);
          
                  Check_Parsed_Names(*specs.ss_var_names, specs.ss_var_do_all_flag);
          
                  if (!xline.empty()) strncpy(line, xline.c_str(), 255);
                  else                strcpy(line, "");
          
                  continue;
                }
              else if ( abbreviation(tok1, "element", 4) &&
                        abbreviation(tok2, "attributes", 3) )
                {
                  xline = Parse_Variables(xline, cmd_file,
                                          specs.elmt_att_do_all_flag,
                                          specs.elmt_att_default,
                                          specs.elmt_att_names,
                                          specs.elmt_att);
          
                  Check_Parsed_Names(*specs.elmt_att_names, specs.elmt_att_do_all_flag);
          
                  if (!xline.empty()) strncpy(line, xline.c_str(), 255);
                  else                strcpy(line, "");
          
                  continue;
                }
              else
                Parse_Die(line);
            }
      
          cmd_file.getline(line, 256);  xline = line;
        }
    }  // end parse command file
  else {
    specs.glob_var_do_all_flag = true;
    specs.node_var_do_all_flag = true;
    specs.elmt_var_do_all_flag = true;
    specs.elmt_att_do_all_flag = true;
    specs.ns_var_do_all_flag = true;
    specs.ss_var_do_all_flag = true;
  }
}

namespace {
  void show_copyright()
  {
    cerr << "\n"
	 << "Copyright(C) 2008 Sandia Corporation.  Under the terms of Contract\n"
	 << "DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains\n"
	 << "certain rights in this software\n"
	 << "\n"
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
	 << "\n"
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
}
