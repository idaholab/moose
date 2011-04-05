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
#include "Tolerance.h"
#include <math.h>
#include "smart_assert.h"

bool Tolerance::use_old_floor = false;

bool Tolerance::Diff(double v1, double v2) const
{
  if (type == IGNORE)
    return false;

  if (use_old_floor) {
    if (fabs(v1 - v2) < floor)
      return false;
  } else {
    if (fabs(v1) <= floor && fabs(v2) <= floor)
      return false;
  }

  if (type == RELATIVE)
    {
      if (v1 == 0.0 && v2 == 0.0) return 0;
      double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
      return fabs(v1 - v2)/max > value;
    }
  else  // absolute diff
    return fabs(v1 - v2) > value;
}

const char* Tolerance::typestr() const
{
  if (type == RELATIVE)
    return "relative";
  else if (type == ABSOLUTE)
    return "absolute";
  else
    return "ignore";
}
    
const char* Tolerance::abrstr() const
{
  if (type == RELATIVE)
    return "rel";
  else if (type == ABSOLUTE)
    return "abs";
  else
    return "ign";
}
    
