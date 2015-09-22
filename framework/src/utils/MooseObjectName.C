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

MooseObjectName::MooseObjectName(const std::string & tag, const std::string & name, const std::string & sep /* = "::" */) :
    _tag(tag),
    _name(name),
    _combined(tag + name),
    _separator(sep)
{
}

MooseObjectName::MooseObjectName(const std::string & sep /* = "::" */) :
    _separator(sep)
{
}

MooseObjectName::MooseObjectName(const MooseObjectName & rhs) :
    _tag(rhs._tag),
    _name(rhs._name),
    _combined(_tag + _name),
    _separator(rhs._separator)
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
