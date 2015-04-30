#include "DerivativeMaterialPropertyNameInterface.h"

#include <sstream>

const std::string
DerivativeMaterialPropertyNameInterface::propertyName(const std::string &base,const std::vector<std::string> &c) const
{
  // to obtain well defined names we sort alphabetically
  std::vector<std::string> a(c);
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

const std::string
DerivativeMaterialPropertyNameInterface::propertyNameFirst(const std::string &base, const std::string &c1) const
{
  return "d" + base + "/d" + c1;
}

const std::string
DerivativeMaterialPropertyNameInterface::propertyNameSecond(const std::string &base, const std::string &c1, const std::string &c2) const
{
  std::vector<std::string> c(2);
  c[0] = c1;
  c[1] = c2;
  return propertyName(base, c);
}

const std::string
DerivativeMaterialPropertyNameInterface::propertyNameThird(const std::string &base, const std::string &c1, const std::string &c2, const std::string &c3) const
{
  std::vector<std::string> c(3);
  c[0] = c1;
  c[1] = c2;
  c[2] = c3;
  return propertyName(base, c);
}
