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
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <vector>
#include <string>
#include <sstream>

#include "stringx.h"

#include "smart_assert.h"
#include "exo_entity.h"
#include "exodusII.h"
#include "util.h"

using namespace std;

namespace {
  template <class T> static std::string to_string(const T & t) {
    std::ostringstream os;
    os << t;
    return os.str();
  }
  int get_index(int file_id,          EXOTYPE exo_type, int id, const char *label);
  int get_num_entities(int file_id,   EXOTYPE exo_type);
  int get_num_variables(int file_id,  EXOTYPE exo_type,         const char *label);
  int get_num_attributes(int file_id, EXOTYPE exo_type, int id, const char *label);
  int get_num_timesteps(int file_id);
}

Exo_Entity::Exo_Entity()
  : fileId(-1),
    id_(EX_INVALID_ID),
    index_(-1),
    numEntity(-1),
    truth_(NULL),
    currentStep(-1),
    numVars(-1),
    results_(NULL),
    numAttr(-1)
{ }

Exo_Entity::Exo_Entity(int file_id, int id)
  : fileId(file_id),
    id_(id),
    index_(-1),
    numEntity(-1),
    truth_(NULL),
    currentStep(-1),
    numVars(-1),
    results_(NULL),
    numAttr(-1)
{
  SMART_ASSERT(file_id > 0);
  SMART_ASSERT(id > EX_INVALID_ID);}


Exo_Entity::Exo_Entity(int file_id, int id, int nnodes)
  : fileId(file_id),
    id_(id),
    index_(-1),
    numEntity(nnodes),
    truth_(NULL),
    currentStep(-1),
    numVars(-1),
    results_(NULL),
    numAttr(-1)
{
  SMART_ASSERT(file_id > 0);
  SMART_ASSERT(id > EX_INVALID_ID);
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
  if (numAttr > 0) {
    for (int i = 0; i < numAttr; ++i)
      delete [] attributes_[i];
  }
}

int Exo_Entity::Check_State() const
{
  SMART_ASSERT(id_ >= EX_INVALID_ID);
  SMART_ASSERT(numEntity >= 0);
  
  SMART_ASSERT( !( id_ == EX_INVALID_ID && numEntity > 0 ) );
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
  if (id_ == EX_INVALID_ID) return "ERROR:  Must initialize block parameters first!";
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
	err = ex_get_var(fileId, time_step, exodus_type(), var_index+1,
			 id_, numEntity, results_[var_index]);
	
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
    // initialize to true for the case of no objects in the block (some older
    // versions of ex_get_object_truth_vector do not set the values at all)
    for (int i = 0; i < numVars; ++i)
      truth_[i] = 1;
    int err = ex_get_object_truth_vector(fileId, exodus_type(), id_, numVars, truth_);
    if (err < 0) {
      std::cerr << "Exo_Entity::get_truth_table(): ex_get_object_truth_vector returned error."
		<< std::endl;
    }
  }
}

string Exo_Entity::Load_Attributes(int attr_index)
{
  SMART_ASSERT(Check_State());
  
  if (fileId < 0) return "ERROR:  Invalid file id!";
  if (id_ == EX_INVALID_ID)   return "ERROR:  Must initialize block parameters first!";
  SMART_ASSERT(attr_index >= 0 && attr_index < numAttr);
  
  if (!attributes_[attr_index] && numEntity) {
    attributes_[attr_index] = new double[numEntity];
    SMART_ASSERT(attributes_[attr_index] != 0);
  }

  if (numEntity) {
    int err = 0;
    err = ex_get_one_attr(fileId, exodus_type(), id_, attr_index+1,
			  attributes_[attr_index]);
	
    if (err < 0) {
      std::cout << "Exo_Entity::Load_Attributes()  ERROR: Call to exodus routine"
		<< " returned error value! " << label() << " id = " << id_ << std::endl;
      std::cout << "Aborting..." << std::endl;
      exit(1);
    }
    else if (err > 0) {
      ostringstream oss;
      oss << "WARNING:  Number " << err
	  << " returned from call to exodus get attribute routine.";
      return oss.str();
    }
  }
  else
    return string("WARNING:  No items in this ") + label();

  return "";
}

const double* Exo_Entity::Get_Attributes(int attr_index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(attr_index >= 0 && attr_index < numAttr);
  return attributes_[attr_index];
}

void Exo_Entity::Free_Attributes()
{
  SMART_ASSERT(Check_State());
  
  for (int v = 0; v < numAttr; ++v) {
    delete [] attributes_[v];
    attributes_[v] = 0;
  }
}

const std::string& Exo_Entity::Get_Attribute_Name(int attr_index) const
{
  SMART_ASSERT(attr_index >= 0 && attr_index < numAttr);
  return attributeNames[attr_index];
}

int Exo_Entity::Find_Attribute_Index(const std::string &name) const
{
  std::string lower_name = name;
  to_lower(lower_name);
  int indx = -1;
  for (int i=0; i < numAttr; i++) {
    if (attributeNames[i] == lower_name) {
      indx = i;
      break;
    }
  }
  return indx;
}

void Exo_Entity::internal_load_params()
{
  numVars = get_num_variables(fileId, exodus_type(), label());
  if (numVars) {
    results_ = new double*[numVars];
    SMART_ASSERT(results_ != 0);
    for (int i = 0; i < numVars; ++i)
      results_[i] = 0;
  }

  numAttr = get_num_attributes(fileId, exodus_type(), id_, label());
  if (numAttr) {
    attributes_.resize(numAttr);

    int name_size = ex_inquire_int(fileId, EX_INQ_MAX_READ_NAME_LENGTH);
    char** names = get_name_array(numAttr, name_size);
    int err = ex_get_attr_names(fileId, exodus_type(), id_, names);
    if (err < 0) {
      std::cout << "ExoII_Read::Get_Init_Data(): ERROR: Failed to get " << label() 
		<< " attribute names!  Aborting..." << std::endl;
      exit(1);
    }

    for (int vg = 0; vg < numAttr; ++vg) {
      SMART_ASSERT(names[vg] != 0);
      if (std::strlen(names[vg]) == 0) {
	std::string name = "attribute_" + to_string(vg+1);
	strncpy(names[vg], name.c_str(), name_size);
      } else if ((int)std::strlen(names[vg]) > name_size) {
	std::cout << "exodiff: ERROR: " << label()
		  << " attribute names appear corrupt\n"
		  << "                A length is 0 or greater than "
		  << "name_size(" << name_size << ")\n"
		  << "                Here are the names that I received from"
		  << " a call to ex_get_attr_names(...):\n";
	for (int k = 1; k <= numAttr; ++k)
	  std::cout << "\t\t" << k << ") \"" << names[k-1] << "\"\n";
	std::cout << "                 Aborting..." << std::endl;
	exit(1);
      }
        
      string n(names[vg]);
      to_lower(n);
      attributeNames.push_back(n);
    }
    free_name_array(names, numAttr);
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
      std::cerr << "ERROR: Invalid entity type in get_num_entities" << std::endl;
      exit(1);
    }
    SMART_ASSERT(inquiry > 0);
    return ex_inquire_int(file_id, inquiry);
  }

  int get_num_variables(int file_id, EXOTYPE type, const char *label)
  {
    int num_vars = 0;  
    int err = ex_get_variable_param(file_id, type, &num_vars);
    if (err < 0) {
      std::cerr << "ERROR: Failed to get number of '" << label 
		<< "' variables!  Aborting..." << std::endl;
      exit(1);
    }
    return num_vars;
  }

  int get_num_attributes(int file_id, EXOTYPE type, int id, const char *label)
  {
    int num_attr = 0;  
    int err = ex_get_attr_param(file_id, type, id, &num_attr);
    if (err < 0) {
      std::cerr << "ERROR: Failed to get number of '" << label 
		<< "' attributes!  Aborting..." << std::endl;
      exit(1);
    }
    return num_attr;
  }

  int get_num_timesteps(int file_id)
  {
    return ex_inquire_int(file_id, EX_INQ_TIME);
  }
}
