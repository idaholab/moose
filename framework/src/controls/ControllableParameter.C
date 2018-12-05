//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ControllableParameter.h"
#include "MooseUtils.h"

void
ControllableParameter::add(ControllableItem * item)
{
  _items.push_back(item);
}

std::string
ControllableParameter::dump() const
{
  std::ostringstream oss;
  for (auto item_ptr : _items)
    oss << item_ptr->dump();
  return oss.str();
}

void
ControllableParameter::checkExecuteOnType(const ExecFlagType & current) const
{
  for (const ControllableItem * const item : _items)
  {
    const std::set<ExecFlagType> & flags = item->getExecuteOnFlags();
    if (!flags.empty() && flags.find(current) == flags.end())
      mooseError("The controllable parameter (",
                 item->name(),
                 ") is not allowed to execute on '",
                 current,
                 "', it is only allowed to execute on '",
                 MooseUtils::join(flags, " "),
                 "'.");
  }
}

std::ostream &
operator<<(std::ostream & stream, const ControllableParameter & obj)
{
  return stream << obj.dump();
}
