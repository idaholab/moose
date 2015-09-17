#include "MooseObjectName.h"

#include <iostream>

MooseObjectName::MooseObjectName(const std::string & tag, const std::string & name, const std::string & sep /* = "::" */) :
    _tag(tag),
    _name(name),
    _combined(tag + name),
    _separator(sep)
{
}

bool
MooseObjectName::operator==(const MooseObjectName & rhs) const
{
  if ( (_name == rhs._name || _name.empty() || rhs._name.empty() ) &&
       (_tag  == rhs._tag  || _tag.empty()  || rhs._tag.empty() ) )
  {
    return true;
  }
  return false;
}

bool
MooseObjectName::operator!=(const MooseObjectName & rhs) const
{
  return !( *this == rhs );
}

bool
MooseObjectName::operator<(const MooseObjectName & rhs) const
{
  return (_combined < rhs._combined);
}

std::ostream &
operator<<(std::ostream & stream, const MooseObjectName & obj)
{
  return stream << obj._tag << obj._separator << obj._name;
}
