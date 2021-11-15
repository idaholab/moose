//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseEnumItem.h"
#include "MooseUtils.h"

MooseEnumItem::MooseEnumItem() : _raw_name("INVALID"), _name("INVALID"), _id(INVALID_ID) {}

MooseEnumItem::MooseEnumItem(const std::string & name, const int & id)
  : _raw_name(MooseUtils::trim(name)), _name(MooseUtils::toUpper(_raw_name)), _id(id)
{
}

MooseEnumItem::MooseEnumItem(const MooseEnumItem & other)
  : _raw_name(other._raw_name), _name(other._name), _id(other._id)
{
}

MooseEnumItem &
MooseEnumItem::operator=(const MooseEnumItem & other)
{
  _raw_name = other._raw_name;
  _name = other._name;
  _id = other._id;
  return *this;
}

bool
MooseEnumItem::operator==(const char * value) const
{
  std::string name(MooseUtils::toUpper(value));
  return _name == name;
}

bool
MooseEnumItem::operator!=(const char * value) const
{
  std::string name(MooseUtils::toUpper(value));
  return _name != name;
}

bool
MooseEnumItem::operator==(const std::string & value) const
{
  std::string name(MooseUtils::toUpper(value));
  return _name == name;
}

bool
MooseEnumItem::operator!=(const std::string & value) const
{
  std::string name(MooseUtils::toUpper(value));
  return _name != name;
}

bool
MooseEnumItem::operator==(const MooseEnumItem & item) const
{
  return _id == item.id() && _name == MooseUtils::toUpper(item.name());
}

bool
MooseEnumItem::operator!=(const MooseEnumItem & item) const
{
  return _id != item.id() && _name != MooseUtils::toUpper(item.name());
}

std::ostream &
operator<<(std::ostream & out, const MooseEnumItem & item)
{
  out << item.rawName();
  return out;
}

void
MooseEnumItem::setID(const int & id)
{
  if (_id != INVALID_ID)
    mooseError("The ID of a MooseEnumItem can not be changed if it is valid, the item ",
               _name,
               " has a valid id of ",
               _id,
               ".");
  _id = id;
}

const int MooseEnumItem::INVALID_ID = std::numeric_limits<int>::min();
