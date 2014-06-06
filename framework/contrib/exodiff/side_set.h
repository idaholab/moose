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

#ifndef SIDE_SET_H
#define SIDE_SET_H

#include "exo_entity.h"
#include <iostream>

template <typename INT>
class ExoII_Read;

template <typename INT>
class Side_Set: public Exo_Entity {
 public:

  Side_Set();
  Side_Set(int file_id, size_t exo_set_id);
  Side_Set(int file_id, size_t exo_set_id, size_t num_sides, size_t num_dist_factors = 0);
  ~Side_Set();

  void apply_map(const INT *node_map);
  const INT* Elements() const;
  const INT* Sides() const;
  std::pair<INT,INT> Side_Id(size_t position) const;
  size_t Side_Index(size_t position) const;

  const double* Distribution_Factors() const;

  void Display_Stats(std::ostream& = std::cout) const;
  void Display      (std::ostream& = std::cout) const;
  int  Check_State() const;

 private:
  Side_Set(const Side_Set&);  // Not written.
  const Side_Set& operator=(const Side_Set&);  // Not written.

  void load_sides(const INT *elmt_map = NULL) const;
  void entity_load_params();

  EXOTYPE exodus_type() const;
  const char* label() const {return "Sideset";}

  size_t num_dist_factors;

  mutable INT*    elmts;
  mutable INT*    sides;
  mutable INT*    sideIndex;
  mutable double* dist_factors;

  friend class ExoII_Read<INT>;
};

#endif
