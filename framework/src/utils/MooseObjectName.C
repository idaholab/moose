//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseObjectName.h"
#include "MooseError.h"

// STL includes
#include <iostream>

MooseObjectName::MooseObjectName(const std::string & tag,
                                 const std::string & name,
                                 const std::string & separator)
  : _tag(tag), _name(name), _combined(tag + name), _separator(separator)
{
  check();
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
    _name = name.erase(0, idx1 + 1);
    _separator = "/";
  }

  // If you get here, just use the supplied name without a tag (this will produce an error in check)
  else
    _name = name;

  check();
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
  if ((_name == rhs._name || _name == "*" || rhs._name == "*") &&
      (_tag == rhs._tag || _tag == "*" || rhs._tag == "*"))
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

void
MooseObjectName::check()
{
  if (_name.empty())
    mooseError("The supplied name cannot be empty, to allow for any name to be supplied use the "
               "'*' character.");
  if (_tag.empty())
    mooseError("The supplied tag cannot be empty, to allow for any tag to be supplied use the '*' "
               "character.");
}
