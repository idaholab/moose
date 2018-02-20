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
#include <cmath>
class DiffData
{
public:
  enum Type
  {
    mm_unknown = 0,
    mm_time = 1,    // Only val and step valid.
    mm_global = 2,  // Only val and step valid.
    mm_nodal = 3,   // Only val, step, and id valid.
    mm_element = 4, // All fields valid for the rest.
    mm_sideset = 5,
    mm_nodeset = 6,
    mm_elematt = 7 // step not valid
  };

  DiffData() : diff(0.0), val1(0.0), val2(0.0), id(0), blk(0), type(mm_unknown) {}

  void set_max(double d, double val_1, double val_2, size_t id_ = 0, size_t blk_ = 0)
  {
    if (diff < d)
    {
      diff = d;
      val1 = val_1;
      val2 = val_2;
      id = id_;
      blk = blk_;
    }
  }

  double diff;
  double val1;
  double val2;
  size_t id;
  size_t blk;

  Type type;
};

class MinMaxData
{
public:
  enum Type
  {
    mm_unknown = 0,
    mm_time = 1,    // Only val and step valid.
    mm_global = 2,  // Only val and step valid.
    mm_nodal = 3,   // Only val, step, and id valid.
    mm_element = 4, // All fields valid for the rest.
    mm_sideset = 5,
    mm_nodeset = 6,
    mm_elematt = 7 // step not valid
  };
  MinMaxData()
    : min_val(DBL_MAX),
      min_step(0),
      min_id(0),
      min_blk(0),
      max_val(-1.0),
      max_step(0),
      max_id(0),
      max_blk(0),
      type(mm_unknown)
  {
  }

  void spec_min_max(double val, int step, size_t id = 0, size_t blk = 0)
  {
    if (std::fabs(val) < min_val)
    {
      min_val = std::fabs(val);
      min_step = step;
      min_id = id;
      min_blk = blk;
    }

    if (std::fabs(val) > max_val)
    {
      max_val = std::fabs(val);
      max_step = step;
      max_id = id;
      max_blk = blk;
    }
  }

  double min_val;
  int min_step;
  size_t min_id;
  size_t min_blk;

  double max_val;
  int max_step;
  size_t max_id;
  size_t max_blk;

  Type type;
};
