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
#ifndef TOLERANCE_H
#define TOLERANCE_H

#include <math.h>

// See http://realtimecollisiondetection.net/blog/?p=89 for a
// description of the COMBINED tolerance.  Basically:
// The absolute tolerance test fails when x and y become large, and
// the relative tolerance test fails when they become small. It is
// therefore desired to combine these two tests together in a single
// test. Over the years at GDC, as well as in my book, I've
// suggested the following combined tolerance test:
//
// if (Abs(x - y) <= EPSILON * Max(1.0f, Abs(x), Abs(y)) ...

enum TOLERANCE_TYPE_enum { RELATIVE = 0, ABSOLUTE = 1, COMBINED = 2, IGNORE = 3, EIGEN_REL = 4, EIGEN_ABS = 5, EIGEN_COM = 6 };

class Tolerance
{
public:
  Tolerance()
    : type(RELATIVE), value(0.0), floor(0.0) {}
  
  Tolerance(TOLERANCE_TYPE_enum tol_type, double tol_value, double tol_floor)
    : type(tol_type), value(tol_value), floor(tol_floor) {}
  
  // Default copy constructor and operator= should work for this simple class...
  
  bool Diff(double v1, double v2) const;

  double Delta(double v1, double v2) const;

  const char* typestr() const;
  const char* abrstr() const;
    
  TOLERANCE_TYPE_enum type;
  double value;
  double floor;

  // If true, use the older defintion of the floor tolerance which was
  // |a-b| < floor.  The new definition is |a| < floor && |b| < floor
  static bool use_old_floor;
};

inline double Tolerance::Delta(double v1, double v2) const
{
  if (type == IGNORE)
    return 0.0;

  double fabv1 = fabs(v1);
  double fabv2 = fabs(v2);
  bool diff = false;
  if (!use_old_floor) {
    if (fabv1 >= floor || fabv2 >= floor) {
      diff = true;
    }
  } else {
    if (fabs(v1 - v2) >= floor) {
      diff = true;
    }
  }

  if (diff) {
    if (type == RELATIVE) {
      if (v1 == 0.0 && v2 == 0.0) return 0.0;
      double max = fabv1 < fabv2 ? fabv2: fabv1;
      return fabs(v1 - v2)/max;
    }
    else if (type == ABSOLUTE) {
      return fabs(v1 - v2);
    }
    else if (type == COMBINED) {
      double max = fabv1 < fabv2 ? fabv2: fabv1;
      if (max > 1.0)
	return fabs(v1 - v2)/max;
      else
	return fabs(v1 - v2);
    }
    else if (type == EIGEN_REL) {
      if (v1 == 0.0 && v2 == 0.0) return 0.0;
      double max = fabv1 < fabv2 ? fabv2: fabv1;
      return fabs(fabv1 - fabv2)/max;
    }
    else if (type == EIGEN_ABS) {
      return fabs(fabv1 - fabv2);
    }
    else if (type == EIGEN_COM) {
      double max = fabv1 < fabv2 ? fabv2: fabv1;
      if (max > 1.0)
	return fabs(fabv1 - fabv2)/max;
      else
	return fabs(fabv1 - fabv2);
    }
  }
  return 0.0;
}

#endif
