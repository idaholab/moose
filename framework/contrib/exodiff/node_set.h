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

#ifndef NODE_SET_H
#define NODE_SET_H

#include "exo_entity.h"
#include "side_set.h" // for Side_Set

template <typename INT>
class ExoII_Read;

template <typename INT>
class Node_Set : public Exo_Entity
{
public:
  Node_Set();
  Node_Set(int file_id, size_t id);
  Node_Set(int file_id, size_t id, size_t nnodes, size_t ndfs = 0);
  ~Node_Set() override;

  void apply_map(const INT * node_map);
  const INT * Nodes() const;
  size_t Node_Id(size_t position) const;
  size_t Node_Index(size_t position) const;

  const double * Distribution_Factors() const;
  void Free_Distribution_Factors() const;

  void Display(std::ostream & /*s*/ = std::cout);
  int  Check_State() const;

private:
  Node_Set(const Node_Set &);                   // Not written.
  const Node_Set & operator=(const Node_Set &); // Not written.

  void entity_load_params() override;

  EXOTYPE exodus_type() const override;
  const char * label() const override { return "Nodeset"; }
  const char * short_label() const override { return "nodeset"; }

  void load_nodes(const INT * node_map = nullptr) const;

  size_t num_dist_factors;

  mutable INT * nodes;           // Array.
  mutable INT * nodeIndex;       // An index array which orders the nodelist in sorted order.
  mutable double * dist_factors; // Array.

  friend class ExoII_Read<INT>;
};

#endif
