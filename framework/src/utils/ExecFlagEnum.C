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

#include "ExecFlagEnum.h"
#include "MooseError.h"

ExecFlagEnum::ExecFlagEnum() : MultiMooseEnum() {}

void
ExecFlagEnum::addAvailableFlags(const std::set<ExecFlagType> & flags)
{
  for (const ExecFlagType & flag : flags)
    addEnumerationName(flag);
}

void
ExecFlagEnum::removeAvailableFlags(const std::set<ExecFlagType> & flags)
{
  for (const auto & item : flags)
    if (find(item) == _items.end())
      mooseError("The supplied item '",
                 item,
                 "' is not an available enum item for the "
                 "MultiMooseEnum object, thus it cannot be removed.");
    else if (contains(item))
      mooseError("The supplied item '", item, "' is a selected item, thus it can not be removed.");

  _items.erase(flags.begin(), flags.end());
}

std::string
ExecFlagEnum::getDocString() const
{
  std::string doc("The list of flag(s) indicating when this object should be executed, the "
                  "available options include \'");
  for (const std::string & name : getNames())
    doc += name + "', '";
  doc.erase(doc.end() - 4, doc.end());
  doc += "').";
  return doc;
}

ExecFlagEnum &
ExecFlagEnum::operator=(const std::initializer_list<ExecFlagType> & flags)
{
  clear();
  for (const ExecFlagType & flag : flags)
    appendCurrent(flag);
  checkDeprecated();
  return *this;
}

ExecFlagEnum &
ExecFlagEnum::operator=(const ExecFlagType & flag)
{
  clear();
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
