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

#include "Specifications.h"

Specifications specs;

using namespace std;

Specifications::Specifications()
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
    coord_sep            (true),
    exit_status_switch   (false),
    dump_mapping         (false),
    show_unmatched       (false),
    noSymmetricNameCheck (false),
    allowNameMismatch    (false),
    doNorms              (false),
    coord_tol            ( ABSOLUTE, 1.0e-6, 0.0 ),
    time_tol             ( RELATIVE, 1.0e-6, 1.0e-15 ),
    time_step_offset     (0),
    time_step_start      (1),
    time_step_stop       (-1),
    time_step_increment  (1),
    max_number_of_names  (DEFAULT_MAX_NUMBER_OF_NAMES),
    default_tol          ( RELATIVE, 1.0e-6, 0.0 ),
    glob_var_names       (0),
    glob_var_do_all_flag (false),
    glob_var             (0),
    node_var_names       (0),
    node_var_do_all_flag (false),
    node_var             (0),
    elmt_var_names       (0),
    elmt_var_do_all_flag (false),
    elmt_var             (0),
    elmt_att_names       (0),
    elmt_att_do_all_flag (false),
    elmt_att             (0),
    ns_var_names         (0),
    ns_var_do_all_flag   (false),
    ns_var               (0),
    ss_var_names         (0),
    ss_var_do_all_flag   (false),
    ss_var               (0),
    num_excluded_steps   (0),
    exclude_steps        (0),
    command_file_name("")
{
  glob_var_default = default_tol;
  node_var_default = default_tol;
  elmt_var_default = default_tol;
  elmt_att_default = default_tol;
  ns_var_default = default_tol;
  ss_var_default = default_tol;
}

void Specifications::allocateNames()
{
  specs.glob_var_names = new vector<string>;
  specs.node_var_names = new vector<string>;
  specs.elmt_var_names = new vector<string>;
  specs.elmt_att_names = new vector<string>;
  specs.ns_var_names   = new vector<string>;
  specs.ss_var_names   = new vector<string>;
}

Specifications::~Specifications()
{
  if (glob_var_names != 0) delete glob_var_names;
  if (node_var_names != 0) delete node_var_names;
  if (elmt_var_names != 0) delete elmt_var_names;
  if (elmt_att_names != 0) delete elmt_att_names;
  if (ns_var_names   != 0) delete ns_var_names;
  if (ss_var_names   != 0) delete ss_var_names;
  
  if (glob_var != 0) delete [] glob_var;
  if (node_var != 0) delete [] node_var;
  if (elmt_var != 0) delete [] elmt_var;
  if (elmt_att != 0) delete [] elmt_att;
  if (ns_var   != 0) delete [] ns_var;
  if (ss_var   != 0) delete [] ss_var;
  
  if (exclude_steps  != 0) delete [] exclude_steps;
}

void Specifications::Set_Max_Names(int size)
{
  if (glob_var != 0) delete [] glob_var;
  if (node_var != 0) delete [] node_var;
  if (elmt_var != 0) delete [] elmt_var;
  if (elmt_att != 0) delete [] elmt_att;
  if (ns_var   != 0) delete [] ns_var;
  if (ss_var   != 0) delete [] ss_var;
  
  max_number_of_names = size;
  
  glob_var  = new Tolerance[max_number_of_names];
  node_var  = new Tolerance[max_number_of_names];
  elmt_var  = new Tolerance[max_number_of_names];
  elmt_att  = new Tolerance[max_number_of_names];
  ns_var    = new Tolerance[max_number_of_names];
  ss_var    = new Tolerance[max_number_of_names];
  
  for (int r = 0; r < max_number_of_names; ++r)
  {
    glob_var[r] = default_tol;
    node_var[r] = default_tol;
    elmt_var[r] = default_tol;
    elmt_att[r] = default_tol;
    ns_var[r]   = default_tol;
    ss_var[r]   = default_tol;
  }
}

void Specifications::allocateExcludeSteps( int num )
{
  num_excluded_steps = num;
  exclude_steps = new int[num];
  for (int i = 0; i < num; ++i)
    exclude_steps[i] = 0;
}
