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
// $Id: exo_entity.C,v 1.1 2008/10/31 05:04:08 gdsjaar Exp $

#include <iostream>
#include <cstdlib>

#include "smart_assert.h"
#include "exo_entity.h"
#include "exodusII.h"
#include "vector"

#include <string>
#include <sstream>

using namespace std;

namespace {
  int get_index(int file_id,         EXOTYPE exo_type, int id, const char *label);
  int get_num_entities(int file_id,  EXOTYPE exo_type);
  int get_num_variables(int file_id, const char *flag, const char *label);
  int get_num_timesteps(int file_id);
  int     inquire_int(int exo_file_id,    int request);
}

Exo_Entity::Exo_Entity()
  : fileId(-1),
    id_(-1),
    index_(-1),
    numEntity(-1),
    truth_(NULL),
    currentStep(-1),
    numVars(-1),
    results_(NULL)
{ }

Exo_Entity::Exo_Entity(int file_id, int id)
  : fileId(file_id),
    id_(id),
    index_(-1),
    numEntity(-1),
    truth_(NULL),
    currentStep(-1),
    numVars(-1),
    results_(NULL)
{
  SMART_ASSERT(file_id > 0);
  SMART_ASSERT(id > 0);
}

Exo_Entity::Exo_Entity(int file_id, int id, int nnodes)
  : fileId(file_id),
    id_(id),
    index_(-1),
    numEntity(nnodes),
    truth_(NULL),
    currentStep(-1),
    numVars(-1),
    results_(NULL)
{
  SMART_ASSERT(file_id > 0);
  SMART_ASSERT(id > 0);
  SMART_ASSERT(nnodes >= 0);
}

Exo_Entity::~Exo_Entity()
{
  delete [] truth_;
  if (numVars > 0) {
    for (int i = 0; i < numVars; ++i)
      delete [] results_[i];
    delete [] results_;
  }
}

int Exo_Entity::Check_State() const
{
  SMART_ASSERT(id_ >= 0);
  SMART_ASSERT(numEntity >= 0);
  
  SMART_ASSERT( !( id_ == 0 && numEntity > 0 ) );
  return 1;
}

void Exo_Entity::initialize(int file_id, int id)
{
  fileId = file_id;
  id_ = id;
  
  index_ = get_index(fileId, exodus_type(), id_, label());

  entity_load_params();

  internal_load_params();
}

bool Exo_Entity::is_valid_var(int var_index) const
{
  SMART_ASSERT(var_index >= 0 && var_index < numVars);
  if (truth_ == NULL) {
    get_truth_table();
  }

  return (truth_[var_index] != 0);
}

string Exo_Entity::Load_Results(int time_step, int var_index)
{
  SMART_ASSERT(Check_State());
  
  if (fileId < 0) return "ERROR:  Invalid file id!";
  if (id_ == 0) return "ERROR:  Must initialize block parameters first!";
  SMART_ASSERT(var_index >= 0 && var_index < numVars);
  
  int num_times = get_num_timesteps(fileId);
  SMART_ASSERT(time_step >= 1 && time_step <= num_times);
  
  if (time_step != currentStep) {
    Free_Results();
    currentStep = time_step;
  }
  
  if (truth_ == NULL) {
    get_truth_table();
  }

  if (truth_[var_index]) {
    if (!results_[var_index] && numEntity) {
      results_[var_index] = new double[numEntity];
      SMART_ASSERT(results_[var_index] != 0);
    }
      if (numEntity) {
	int err = 0;
	switch (exodus_type()) {
	case EX_ELEM_BLOCK:
	  err = ex_get_elem_var(fileId, time_step, var_index+1,
                                id_, numEntity, results_[var_index]);
	  break;
	case EX_NODE_SET:
	  err = ex_get_nset_var(fileId, time_step, var_index+1,
				id_, numEntity, results_[var_index]);
	  break;
	case EX_SIDE_SET:
	  err = ex_get_sset_var(fileId, time_step, var_index+1,
				id_, numEntity, results_[var_index]);
	  break;
	default:
	  std::cerr << "INTERNAL ERROR: Incorrect exodus type in Exo_Entity::Load_Results()\n";
	  exit(1);
	}

	if (err < 0) {
	  std::cout << "Exo_Entity::Load_Results()  ERROR: Call to exodus routine"
		    << " returned error value! " << label() << " id = " << id_ << std::endl;
	  std::cout << "Aborting..." << std::endl;
	  exit(1);
	}
	else if (err > 0) {
          ostringstream oss;
          oss << "WARNING:  Number " << err
              << " returned from call to exodus get variable routine.";
	  return oss.str();
        }
      }
      else
	return string("WARNING:  No items in this ") + label();
    }
  else {
    return string("WARNING: Variable not stored in this ") + label();
  }
  return "";
}

const double* Exo_Entity::Get_Results(int var_index) const
{
  SMART_ASSERT(Check_State());
  if (currentStep == 0) return 0;
  SMART_ASSERT(var_index >= 0 && var_index < numVars);
  return results_[var_index];
}

void Exo_Entity::Free_Results()
{
  SMART_ASSERT(Check_State());
  
  currentStep = 0;
  for (int v = 0; v < numVars; ++v) {
    delete [] results_[v];
    results_[v] = 0;
  }
}

void Exo_Entity::get_truth_table() const
{
  if (numVars > 0 && truth_ == NULL) {
    truth_ = new int[numVars]; SMART_ASSERT(truth_ != NULL);
#if EX_API_VERS_NODOT > 467
    int err = ex_get_object_truth_vector(fileId, exodus_type(), id_, numVars, truth_);
#else
    int err = ex_get_object_truth_vector(fileId, exodus_flag(), id_, numVars, truth_);
#endif    
    if (err < 0) {
      std::cerr << "Exo_Entity::get_truth_table(): ex_get_object_truth_vector returned error."
		<< std::endl;
    }
  }
}

void Exo_Entity::internal_load_params()
{
  numVars = get_num_variables(fileId, exodus_flag(), label());
  if (numVars) {
    results_ = new double*[numVars];
    SMART_ASSERT(results_ != 0);
    for (int i = 0; i < numVars; ++i)
      results_[i] = 0;
  }
}

namespace {
  int get_index(int file_id, EXOTYPE exo_type, int id, const char *label)
  {
    // Get ids...
    int count = get_num_entities(file_id, exo_type);
    std::vector<int> ids(count);

    ex_get_ids(file_id, exo_type, &ids[0]);

    for (int i=0; i<count; i++) {
      if (ids[i] == id)
	return i;
    }

    std::cerr << "ERROR:  " << label << " id " <<id << " does not exist!\n";
    return -1;
  }

  int get_num_entities(int file_id, EXOTYPE exo_type)
  {
    int inquiry = 0;
    switch (exo_type) {
    case EX_ELEM_BLOCK:
      inquiry = EX_INQ_ELEM_BLK;
      break;
    case EX_NODE_SET:
      inquiry = EX_INQ_NODE_SETS;
      break;
    case EX_SIDE_SET:
      inquiry = EX_INQ_SIDE_SETS;
      break;
    default:
      ; // Make compiler not complain about "unhandled" enumerations
    }
    SMART_ASSERT(inquiry > 0);
    return inquire_int(file_id, inquiry);
  }

  int get_num_variables(int file_id, const char *type, const char *label)
  {
    int num_vars = 0;  
    int err = ex_get_var_param(file_id, type, &num_vars);
    if (err < 0) {
      std::cerr << "ERROR: Failed to get number of '" << label 
		<< "' variables!  Aborting..." << std::endl;
      exit(1);
    }
    return num_vars;
  }

  int get_num_timesteps(int file_id)
  {
    return inquire_int(file_id, EX_INQ_TIME);
  }

  int inquire_int(int exo_file_id, int request)
  {
    SMART_ASSERT(exo_file_id >= 0);
    int   get_int = 0;
    float get_flt = 0.0;
    char  get_str[MAX_LINE_LENGTH+1];

    int err = ex_inquire(exo_file_id, request, &get_int, &get_flt, get_str);

    if (err < 0) {
      std::cout << "ExoII_Read::Inquire_int(): ERROR " << err
		<< ": ex_inquire failed!  Aborting..." << std::endl;
      exit(1);
    }

    if (err > 0)
      std::cout << "ExoII_Read::Inquire_int(): WARNING: " << err
		<< " issued by ex_inquire call!" << std::endl;

    return get_int;
  }

}
