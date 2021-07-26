//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DerivativeMaterialPropertyNameInterface.h"

#include <sstream>
#include <algorithm>

const MaterialPropertyName
DerivativeMaterialPropertyNameInterface::derivativePropertyName(
    const MaterialPropertyName & base, const std::vector<SymbolName> & c) const
{
  // to obtain well defined names we sort alphabetically
  std::vector<SymbolName> a(c);
  std::sort(a.begin(), a.end());

  // derivative order
  unsigned int order = a.size();
  if (order == 0)
    return base;

  // build the property name as a stringstream
  std::stringstream name;

  // build numerator
  name << 'd';
  if (order > 1)
    name << '^' << order;
  name << base << '/';

  // build denominator with 'pretty' names using exponents rather than repeat multiplication
  unsigned int exponent = 1;
  for (unsigned i = 1; i <= order; ++i)
  {
    if (i == order || a[i - 1] != a[i])
    {
      name << 'd' << a[i - 1];
      if (exponent > 1)
        name << '^' << exponent;
      exponent = 1;
    }
    else
      exponent++;
  }

  return name.str();
}

const MaterialPropertyName
DerivativeMaterialPropertyNameInterface::derivativePropertyNameFirst(
    const MaterialPropertyName & base, const SymbolName & c1) const
{
  return "d" + base + "/d" + c1;
}

const MaterialPropertyName
DerivativeMaterialPropertyNameInterface::derivativePropertyNameSecond(
    const MaterialPropertyName & base, const SymbolName & c1, const SymbolName & c2) const
{
  return derivativePropertyName(base, {c1, c2});
}

const MaterialPropertyName
DerivativeMaterialPropertyNameInterface::derivativePropertyNameThird(
    const MaterialPropertyName & base,
    const SymbolName & c1,
    const SymbolName & c2,
    const SymbolName & c3) const
{
  return derivativePropertyName(base, {c1, c2, c3});
}
