#include "DerivativeMaterialPropertyNameInterface.h"

#include <sstream>
#include <algorithm>

const MaterialPropertyName
DerivativeMaterialPropertyNameInterface::propertyName(const MaterialPropertyName &base,const std::vector<VariableName> &c) const
{
  // to obtain well defined names we sort alphabetically
  std::vector<VariableName> a(c);
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
    if (i == order || a[i-1] != a[i])
    {
      name << 'd' << a[i-1];
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
DerivativeMaterialPropertyNameInterface::propertyNameFirst(const MaterialPropertyName &base, const VariableName &c1) const
{
  return "d" + base + "/d" + c1;
}

const MaterialPropertyName
DerivativeMaterialPropertyNameInterface::propertyNameSecond(const MaterialPropertyName &base, const VariableName &c1, const VariableName &c2) const
{
  std::vector<VariableName> c(2);
  c[0] = c1;
  c[1] = c2;
  return propertyName(base, c);
}

const MaterialPropertyName
DerivativeMaterialPropertyNameInterface::propertyNameThird(const MaterialPropertyName &base, const VariableName &c1, const VariableName &c2, const VariableName &c3) const
{
  std::vector<VariableName> c(3);
  c[0] = c1;
  c[1] = c2;
  c[2] = c3;
  return propertyName(base, c);
}
