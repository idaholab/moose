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
#ifndef ED_NORM_H
#define ED_NORM_H

#include <cmath>
class Norm
{
public:
  Norm()
    : l1_norm_1(0.0), l1_norm_2(0.0), l1_norm_d(0.0), l2_norm_1(0.0), l2_norm_2(0.0), l2_norm_d(0.0)
  {
  }

  double diff(int order) const
  {
    if (order == 1)
    {
      return l1_norm_d;
    }
    if (order == 2)
    {
      return std::sqrt(l2_norm_d);
    }
    else
    {
      return 0.0;
    }
  }

  double left(int order) const
  {
    if (order == 1)
    {
      return l1_norm_1;
    }
    if (order == 2)
    {
      return std::sqrt(l2_norm_1);
    }
    else
    {
      return 0.0;
    }
  }

  double right(int order) const
  {
    if (order == 1)
    {
      return l1_norm_2;
    }
    if (order == 2)
    {
      return std::sqrt(l2_norm_2);
    }
    else
    {
      return 0.0;
    }
  }

  double relative(int order) const
  {
    double l = left(order);
    double r = right(order);
    double lr_max = l > r ? l : r;
    return diff(order) / lr_max;
  }

  void add_value(double val1, double val2)
  {
    l1_norm_d += std::fabs(val1 - val2);
    l1_norm_1 += std::fabs(val1);
    l1_norm_2 += std::fabs(val2);

    l2_norm_d += (val1 - val2) * (val1 - val2);
    l2_norm_1 += val1 * val1;
    l2_norm_2 += val2 * val2;
  }

  double l1_norm_1;
  double l1_norm_2;
  double l1_norm_d;

  double l2_norm_1;
  double l2_norm_2;
  double l2_norm_d;
};

#endif
