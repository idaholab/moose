/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "MooseObjectName.h"

// STL includes
#include <iostream>

MooseObjectName::MooseObjectName(const std::string & tag,
                                 const std::string & name,
                                 const std::string & separator)
  : _tag(tag), _name(name), _combined(tag + name), _separator(separator)
{
}

MooseObjectName::MooseObjectName(std::string name) : _separator("::")
{
  // Tags may be separated by a :: or the last /
  std::size_t idx0 = name.find("::");
  std::size_t idx1 = name.rfind("/");

  // Case when :: is found
  if (idx0 != std::string::npos)
  {
    _tag = name.substr(0, idx0);
    _name = name.erase(0, idx0 + 2);
  }

  // Case when a / is found
  else if (idx1 != std::string::npos)
  {
    _tag = name.substr(0, idx1);
    _name = name.erase(0, idx1 + 2);
    _separator = "/";
  }

  // If you get here, just use the supplied name without a tag
  else
    _name = name;

  _combined = _tag + _name;
}

MooseObjectName::MooseObjectName() : _separator("/") {}

MooseObjectName::MooseObjectName(const MooseObjectName & rhs)
  : _tag(rhs._tag), _name(rhs._name), _combined(rhs._combined), _separator(rhs._separator)
{
}

bool
MooseObjectName::operator==(const MooseObjectName & rhs) const
{
  if ((_name == rhs._name || _name.empty() || rhs._name.empty()) &&
      (_tag == rhs._tag || _tag.empty() || rhs._tag.empty()))
  {
    return true;
  }
  return false;
}

bool
MooseObjectName::operator!=(const MooseObjectName & rhs) const
{
  return !(*this == rhs);
}

bool
MooseObjectName::operator<(const MooseObjectName & rhs) const
{
  return (_combined < rhs._combined);
}

std::ostream &
operator<<(std::ostream & stream, const MooseObjectName & obj)
{
  if (obj._tag.empty())
    return stream << obj._name;
  else
    return stream << obj._tag << obj._separator << obj._name;
}
