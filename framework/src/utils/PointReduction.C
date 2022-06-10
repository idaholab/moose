//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointReduction.h"
#include "MooseUtils.h"
#include "MooseError.h"

#include <algorithm>
#include <cmath>

namespace PointReduction
{

Real
sqr(Real a)
{
  mooseDeprecated("use PointReduction::square() instead");
  return a * a;
}

Real
square(Real a)
{
  return a * a;
}

Real
perpendicularDistance(const FunctionNode & node,
                      const FunctionNode & begin,
                      const FunctionNode & end)
{
  const Real x0 = node.first;
  const Real y0 = node.second;
  const Real x1 = begin.first;
  const Real y1 = begin.second;
  const Real x2 = end.first;
  const Real y2 = end.second;

  const Real denom = std::sqrt(square(y2 - y1) + square(x2 - x1));
  mooseAssert(MooseUtils::absoluteFuzzyGreaterThan(denom, 0.0),
              "Line begin and end points must not be the same");

  return std::abs((y2 - y1) * x0 - (x2 - x1) * y0 + x2 * y1 - y2 * x1) / denom;
}

void
douglasPeuckerRecurse(const FunctionNodeList & list,
                      const Real epsilon,
                      std::vector<bool> & keep,
                      std::size_t begin,
                      std::size_t end)
{
  // Find the point with the maximum distance from the line defined by begin and end
  Real dmax = 0.0;
  std::size_t index = 0;

  for (std::size_t i = begin; i <= end; ++i)
    if (keep[i])
    {
      const Real d = perpendicularDistance(list[i], list[begin], list[end]);
      if (d > dmax)
      {
        index = i;
        dmax = d;
      }
    }

  // If max distance is greater than epsilon, recursively simplify
  if (dmax > epsilon)
  {
    // Recursive call
    douglasPeuckerRecurse(list, epsilon, keep, begin, index);
    douglasPeuckerRecurse(list, epsilon, keep, index, end);
  }
  else
  {
    // remove all points between begin and end
    for (std::size_t i = begin + 1; i < end; ++i)
      keep[i] = false;
  }
}

FunctionNodeList
douglasPeucker(const FunctionNodeList & list, const Real epsilon)
{
  // set up keep list for function nodes
  std::vector<bool> keep(list.size(), true);
  douglasPeuckerRecurse(list, epsilon, keep, 0, list.size() - 1);

  FunctionNodeList result;
  result.reserve(std::count_if(keep.begin(), keep.end(), [](bool k) { return k; }));

  /// filter result
  for (std::size_t i = 0; i < list.size(); ++i)
    if (keep[i])
      result.push_back(list[i]);

  return result;
}

} // namespace PointReduction
