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
class DiffData
{
 public:
  enum Type
    { mm_unknown = 0,
      mm_time    = 1, // Only val and step valid.
      mm_global  = 2, // Only val and step valid.
      mm_nodal   = 3, // Only val, step, and id valid.
      mm_element = 4, // All fields valid for the rest.
      mm_sideset = 5,
      mm_nodeset = 6,
      mm_elematt = 7  // step not valid
    };
  
  DiffData()
    : diff(0.0), val1(0.0), val2(0.0), id(-1), blk(-1), type(mm_unknown)
    {}
    
    void set_max(double d, double val_1, double val_2, int id_=-1, int blk_=-1)
    {
      if (diff < d) {
	diff = d;
	val1 = val_1;
	val2 = val_2;
	if (id_  != -1) id  = id_;
	if (blk_ != -1) blk = blk_;
      }
    }
    
    double diff;
    double val1;
    double val2;
    int    id;
    int    blk;
    
    Type   type;
};

class MinMaxData
{
 public:
  enum Type
    { mm_unknown = 0,
      mm_time    = 1, // Only val and step valid.
      mm_global  = 2, // Only val and step valid.
      mm_nodal   = 3, // Only val, step, and id valid.
      mm_element = 4, // All fields valid for the rest.
      mm_sideset = 5,
      mm_nodeset = 6,
      mm_elematt = 7  // step not valid
    };
  MinMaxData()
    : min_val(DBL_MAX), min_step(-1), min_id(-1), min_blk(-1), 
    max_val(-1.0),    max_step(-1), max_id(-1), max_blk(-1),
    type(mm_unknown)
      {}

    void spec_min_max(double val, int step, int id=-1, int blk=-1)
    {
      if (fabs(val) < min_val) {
	min_val = fabs(val);
	min_step = step;
	if (id  != -1) min_id  = id;
	if (blk != -1) min_blk = blk;
      }
    
      if (fabs(val) > max_val) {
	max_val = fabs(val);
	max_step = step;
	if (id  != -1) max_id  = id;
	if (blk != -1) max_blk = blk;
      }
    }

    double min_val;
    int    min_step;
    int    min_id;
    int    min_blk;

    double max_val;
    int    max_step;
    int    max_id;
    int    max_blk;

    Type   type;
};
