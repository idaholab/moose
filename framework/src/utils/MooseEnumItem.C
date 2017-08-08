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
#include "MooseEnumItem.h"
#include "MooseUtils.h"

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
MooseEnumItem::operator==(const MooseEnumItem &) const
{
  mooseError("Direct comparison between MooseEnumItems is not supported.");
}

bool
MooseEnumItem::operator!=(const MooseEnumItem &) const
{
  mooseError("Direct comparison between MooseEnumItems is not supported.");
}

std::ostream &
operator<<(std::ostream & out, const MooseEnumItem & item)
{
  out << item.rawName();
  return out;
}
