//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGEngUnitList.h"

namespace CSG
{

CSGEngUnitList::CSGEngUnitList() {}

CSGEngUnit &
CSGEngUnitList::addEngUnit(CSGEngUnit & unit)
{
  const auto & name = unit.getName();
  if (hasEngUnit(name))
    mooseError("An engineering unit with name '", name, "' already exists in geometry.");
  _eng_units.push_back(&unit);
  return unit;
}

CSGEngUnit &
CSGEngUnitList::getEngUnit(const std::string & name) const
{
  for (auto * ptr : _eng_units)
    if (ptr->getName() == name)
      return *ptr;
  mooseError("Engineering unit with name '", name, "' does not exist in this CSGBase.");
}

void
CSGEngUnitList::removeEngUnit(const CSGEngUnit & unit)
{
  auto it = std::find(_eng_units.begin(), _eng_units.end(), &unit);
  if (it != _eng_units.end())
    _eng_units.erase(it);
}

std::vector<std::reference_wrapper<const CSGEngUnit>>
CSGEngUnitList::getAllEngUnits() const
{
  std::vector<std::reference_wrapper<const CSGEngUnit>> result;
  for (const auto * ptr : _eng_units)
    result.push_back(*ptr);
  return result;
}

std::vector<std::reference_wrapper<const CSGSurfaceEngUnit>>
CSGEngUnitList::getAllSurfaceEngUnits() const
{
  std::vector<std::reference_wrapper<const CSGSurfaceEngUnit>> result;
  for (const auto * ptr : _eng_units)
    if (const auto * surf = dynamic_cast<const CSGSurfaceEngUnit *>(ptr))
      result.push_back(*surf);
  return result;
}

std::vector<std::reference_wrapper<const CSGCellEngUnit>>
CSGEngUnitList::getAllCellEngUnits() const
{
  std::vector<std::reference_wrapper<const CSGCellEngUnit>> result;
  for (const auto * ptr : _eng_units)
    if (const auto * cell = dynamic_cast<const CSGCellEngUnit *>(ptr))
      result.push_back(*cell);
  return result;
}

std::vector<std::reference_wrapper<const CSGUniverseEngUnit>>
CSGEngUnitList::getAllUniverseEngUnits() const
{
  std::vector<std::reference_wrapper<const CSGUniverseEngUnit>> result;
  for (const auto * ptr : _eng_units)
    if (const auto * univ = dynamic_cast<const CSGUniverseEngUnit *>(ptr))
      result.push_back(*univ);
  return result;
}

bool
CSGEngUnitList::operator==(const CSGEngUnitList & other) const
{
  const auto all_units = this->getAllEngUnits();
  const auto other_units = other.getAllEngUnits();

  if (all_units.size() != other_units.size())
    return false;

  for (const auto & unit : all_units)
  {
    const std::string & unit_name = unit.get().getName();
    const auto & other_unit = other.getEngUnit(unit_name);
    if (unit.get() != other_unit)
      return false;
  }
  return true;
}

bool
CSGEngUnitList::operator!=(const CSGEngUnitList & other) const
{
  return !(*this == other);
}

} // namespace CSG
