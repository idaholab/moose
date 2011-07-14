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

  if (type == RELATIVE) {
    if (v1 == 0.0 && v2 == 0.0) return 0;
    double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
    return fabs(v1 - v2) > value * max;

  } else if (type == ABSOLUTE) {
    return fabs(v1 - v2) > value;

  } else if (type == COMBINED) {
    // if (Abs(x - y) <= Max(absTol, relTol * Max(Abs(x), Abs(y))))
    // In the current implementation, absTol == relTol;
    // At some point, store both values...
    // In summary, use abs tolerance if both values are less than 1.0;
    // else use relative tolerance.

    double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
    double tol = 1.0 < max ? max : 1.0;
    return fabs(v1 - v2) >= tol * value;

    // EIGEN type diffs work on the absolute values. They are intended
    // for diffing eigenvectors where one shape may be the inverse of
    // the other and they should compare equal.  Eventually would like
    // to do a better check to ensure that ratio of one shape to other
    // is 1 or -1...
  } else if (type == EIGEN_REL) {
    if (v1 == 0.0 && v2 == 0.0) return 0;
    double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
    return fabs(fabs(v1) - fabs(v2)) > value * max;

  } else if (type == EIGEN_ABS) {
    return fabs(fabs(v1) - fabs(v2)) > value;

  } else if (type == EIGEN_COM) {
    // if (Abs(x - y) <= Max(absTol, relTol * Max(Abs(x), Abs(y))))
    // In the current implementation, absTol == relTol;
    // At some point, store both values...
    // In summary, use abs tolerance if both values are less than 1.0;
    // else use relative tolerance.

    double max = fabs(v1) < fabs(v2) ? fabs(v2): fabs(v1);
    double tol = 1.0 < max ? max : 1.0;
    return fabs(fabs(v1) - fabs(v2)) >= tol * value;

  } else {
    return true;
  }
}

const char* Tolerance::typestr() const
{
  if (type == RELATIVE)
    return "relative";
  else if (type == ABSOLUTE)
    return "absolute";
  else if (type == COMBINED)
    return "combined";
  else if (type == EIGEN_REL)
    return "eigenrel";
  else if (type == EIGEN_ABS)
    return "eigenabs";
  else if (type == EIGEN_COM)
    return "eigencom";
  else
    return "ignore";
}
    
const char* Tolerance::abrstr() const
{
  if (type == RELATIVE)
    return "rel";
  else if (type == ABSOLUTE)
    return "abs";
  else if (type == COMBINED)
    return "com";
  else if (type == EIGEN_REL)
    return "ere";
  else if (type == EIGEN_ABS)
    return "eab";
  else if (type == EIGEN_COM)
    return "eco";
  else
    return "ign";
}
    
