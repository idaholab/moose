//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ControllableItem.h"
#include "ConsoleUtils.h"

ControllableItem::ControllableItem(const MooseObjectParameterName & name,
                                   libMesh::Parameters::Value * value,
                                   const std::set<ExecFlagType> & flags)
  : _execute_flags(flags)
{
  _pairs.emplace_back(name, value);
}

ControllableItem::ControllableItem() {}

void
ControllableItem::connect(ControllableItem * item, bool type_check)
{
  for (const auto & pair : item->_pairs)
  {
    if (type_check && type() != pair.second->type())
      mooseError("The master parameter (",
                 name(),
                 ") has a type '",
                 type(),
                 "' and cannot be connected to the parameter (",
                 pair.first,
                 ") with a different type of '",
                 pair.second->type(),
                 "'.");

    _pairs.emplace_back(pair.first, pair.second);
  }
}

std::string
ControllableItem::dump(unsigned int indent /*=0*/) const
{

  // Count of objects, for printing a number with the parameter
  unsigned int index = 0;

  std::ostringstream oss;
  for (const auto & pair : _pairs)
  {
    if (index == 0) // master parameter
      oss << ConsoleUtils::indent(indent) << COLOR_GREEN << pair.first << COLOR_DEFAULT << " = ";
    else // secondary parameters
      oss << ConsoleUtils::indent(indent + 2) << COLOR_YELLOW << pair.first << COLOR_DEFAULT
          << " = ";

    pair.second->print(oss);
    oss << " <" << pair.second->type() << ">" << '\n';
    index++;
  }
  return oss.str();
}

std::string
ControllableItem::type() const
{
  return _pairs[0].second->type();
}

const MooseObjectParameterName &
ControllableItem::name() const
{
  return _pairs[0].first;
}

ControllableAlias::ControllableAlias(const MooseObjectParameterName & name, ControllableItem * item)
  : ControllableItem(), _name(name)
{
  connect(item, false);
}

const MooseObjectParameterName &
ControllableAlias::name() const
{
  return _name;
}

std::string
ControllableAlias::dump(unsigned int indent /*=0*/) const
{
  // The output stream
  std::ostringstream oss;
  oss << ConsoleUtils::indent(indent) << COLOR_GREEN << _name << COLOR_DEFAULT;
  for (const auto & pair : _pairs)
  {
    oss << ConsoleUtils::indent(indent + 2) << COLOR_YELLOW << pair.first << COLOR_DEFAULT << " = ";
    pair.second->print(oss);
    oss << " <" << pair.second->type() << ">\n";
  }
  return oss.str();
}

std::ostream &
operator<<(std::ostream & stream, const ControllableItem & obj)
{
  return stream << obj.dump();
}
