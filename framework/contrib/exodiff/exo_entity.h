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

#ifndef EXO_ENTITY_H
#define EXO_ENTITY_H

#include "libmesh/exodusII.h"
#include <iostream>
#include <string>
#include <vector>

#if defined(EX_API_VERS_NODOT)
#if EX_API_VERS_NODOT > 467
using EXOTYPE = ex_entity_type;
#else
typedef int EXOTYPE;
#endif
#else
typedef int EXOTYPE;
#endif

template <typename INT>
class ExoII_Read;

class Exo_Entity
{
public:
  Exo_Entity();
  Exo_Entity(int file_id, size_t id);
  Exo_Entity(int file_id, size_t id, size_t nnodes);
  virtual ~Exo_Entity();

  size_t Size() const { return numEntity; }

  size_t Id() const { return id_; }
  size_t Index() const { return index_; }

  void Display_Stats(std::ostream & = std::cout);
  void Display(std::ostream & = std::cout);
  int Check_State() const;

  void initialize(int file_id, size_t id);

  bool is_valid_var(size_t var_index) const;
  size_t var_count() const { return numVars; }
  std::string Load_Results(int time_step, int var_index);
  std::string Load_Results(int t1, int t2, double proportion, int var_index); // Interpolation

  const double * Get_Results(int var_index) const;
  void Free_Results();

  int attr_count() const { return numAttr; }
  std::string Load_Attributes(int attr_index);
  const double * Get_Attributes(int attr_index) const;
  void Free_Attributes();

  const std::string & Get_Attribute_Name(int attr_index) const;
  const std::string & Name() const { return name_; }
  const std::vector<std::string> & Attribute_Names() const { return attributeNames; }
  int Find_Attribute_Index(const std::string & name) const;

protected:
  std::string name_;
  int fileId;
  ex_entity_id id_;
  size_t index_;    // 0-offset index into Exodus nodeset list.
  size_t numEntity; // Number of items (nodes, sides, elements)

private:
  Exo_Entity(const Exo_Entity &);                   // Not written.
  const Exo_Entity & operator=(const Exo_Entity &); // Not written.

  virtual void entity_load_params() = 0;
  void internal_load_params();

  // Return "Element Block", "Nodeset", "Sideset, depending on underlying type.
  virtual const char * label() const = 0;

  // Return "block", "nodelist", "surface", depending on underlying type.
  virtual const char * short_label() const = 0;

  // Return EX_ELEM_BLOCK, EX_NODE_SET, ... of underlying type
  virtual EXOTYPE exodus_type() const = 0;

  void get_truth_table() const;

  mutable int * truth_;              // Array; holds local truth table for this entity
  int currentStep;                   // Time step number of the current results.
  int numVars;                       // Total number of variables in the file.
  double ** results_;                // Array of pointers (length numVars)
                                     // to arrays of results (length num_entity).
  int numAttr;                       // Total number of attributes in the file.
  std::vector<double *> attributes_; // Array of pointers (length numAttr)
                                     // to arrays of attributes (length num_entity).
  std::vector<std::string> attributeNames;

  template <typename INT>
  friend class ExoII_Read;
};
#endif
