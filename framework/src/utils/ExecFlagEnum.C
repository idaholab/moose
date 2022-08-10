//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecFlagEnum.h"
#include "MooseError.h"
#include "Conversion.h"

ExecFlagEnum::ExecFlagEnum() : MultiMooseEnum() {}
ExecFlagEnum::ExecFlagEnum(const MultiMooseEnum & other) : MultiMooseEnum(other) {}
ExecFlagEnum::ExecFlagEnum(const ExecFlagEnum & other) : MultiMooseEnum(other) {}

const ExecFlagType &
ExecFlagEnum::addAvailableFlags(const ExecFlagType & flag)
{
  return addEnumerationItem(flag);
}

void
ExecFlagEnum::removeAvailableFlags(const ExecFlagType & flag)
{
  if (find(flag) == _items.end())
    mooseError("The supplied item '",
               flag,
               "' is not an available enum item for the "
               "MultiMooseEnum object, thus it cannot be removed.");
  else if (contains(flag))
    mooseError("The supplied item '", flag, "' is a selected item, thus it can not be removed.");

  _items.erase(flag);
}

std::string
ExecFlagEnum::getDocString() const
{
  std::string doc("The list of flag(s) indicating when this object should be executed, the "
                  "available options include ");
  doc += Moose::stringify(getNames(), ", ");
  doc += ".";
  return doc;
}

ExecFlagEnum &
ExecFlagEnum::operator=(const std::initializer_list<ExecFlagType> & flags)
{
  clear();
  *this += flags;
  return *this;
}

ExecFlagEnum &
ExecFlagEnum::operator=(const ExecFlagType & flag)
{
  clear();
  *this += flag;
  return *this;
}

ExecFlagEnum &
ExecFlagEnum::operator+=(const std::initializer_list<ExecFlagType> & flags)
{
  for (const ExecFlagType & flag : flags)
    appendCurrent(flag);
  checkDeprecated();
  return *this;
}

ExecFlagEnum &
ExecFlagEnum::operator+=(const ExecFlagType & flag)
{
  appendCurrent(flag);
  checkDeprecated();
  return *this;
}

void
ExecFlagEnum::appendCurrent(const ExecFlagType & item)
{
  if (find(item) == _items.end())
    mooseError("The supplied item '",
               item,
               "' is not an available item for the "
               "ExecFlagEnum object, thus it cannot be set as current.");
  _current.push_back(item);
}
