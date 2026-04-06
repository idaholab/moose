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
CSGEngUnitList::addEngUnit(std::unique_ptr<CSGEngUnit> unit)
{
  auto name = unit->getName();
  auto [it, inserted] = _eng_units.emplace(name, std::move(unit));
  if (!inserted)
    mooseError("An engineering unit with name '", name, "' already exists in geometry.");
  return *it->second;
}

CSGEngUnit &
CSGEngUnitList::getEngUnit(const std::string & name) const
{
  auto it = _eng_units.find(name);
  if (it == _eng_units.end())
    mooseError("Engineering unit with name '", name, "' does not exist in this CSGBase.");
  return *it->second;
}

std::vector<std::reference_wrapper<const CSGEngUnit>>
CSGEngUnitList::getAllEngUnits() const
{
  std::vector<std::reference_wrapper<const CSGEngUnit>> result;
  for (const auto & [name, unit_ptr] : _eng_units)
    result.push_back(*unit_ptr);
  return result;
}

std::vector<std::reference_wrapper<const CSGSurfaceEngUnit>>
CSGEngUnitList::getAllSurfaceEngUnits() const
{
  std::vector<std::reference_wrapper<const CSGSurfaceEngUnit>> result;
  for (const auto & [name, unit_ptr] : _eng_units)
  {
    const CSGSurfaceEngUnit * surf = dynamic_cast<const CSGSurfaceEngUnit *>(unit_ptr.get());
    if (surf)
      result.push_back(*surf);
  }
  return result;
}

std::vector<std::reference_wrapper<const CSGCellEngUnit>>
CSGEngUnitList::getAllCellEngUnits() const
{
  std::vector<std::reference_wrapper<const CSGCellEngUnit>> result;
  for (const auto & [name, unit_ptr] : _eng_units)
  {
    const CSGCellEngUnit * cell = dynamic_cast<const CSGCellEngUnit *>(unit_ptr.get());
    if (cell)
      result.push_back(*cell);
  }
  return result;
}

std::vector<std::reference_wrapper<const CSGUniverseEngUnit>>
CSGEngUnitList::getAllUniverseEngUnits() const
{
  std::vector<std::reference_wrapper<const CSGUniverseEngUnit>> result;
  for (const auto & [name, unit_ptr] : _eng_units)
  {
    const CSGUniverseEngUnit * univ = dynamic_cast<const CSGUniverseEngUnit *>(unit_ptr.get());
    if (univ)
      result.push_back(*univ);
  }
  return result;
}

void
CSGEngUnitList::renameEngUnit(const CSGEngUnit & unit, const std::string & name)
{
  const auto & old_name = unit.getName();
  auto it = _eng_units.find(old_name);
  if (it == _eng_units.end() || it->second.get() != &unit)
    mooseError("Engineering unit '",
               old_name,
               "' cannot be renamed as it does not exist in this CSGBase instance.");

  auto unit_ptr = std::move(it->second);
  _eng_units.erase(old_name);

  if (auto * surf = dynamic_cast<CSGSurfaceEngUnit *>(unit_ptr.get()))
    static_cast<CSGSurface *>(surf)->setName(name);
  else if (auto * cell = dynamic_cast<CSGCellEngUnit *>(unit_ptr.get()))
    static_cast<CSGCell *>(cell)->setName(name);
  else if (auto * univ = dynamic_cast<CSGUniverseEngUnit *>(unit_ptr.get()))
    static_cast<CSGUniverse *>(univ)->setName(name);
  else
    mooseError("Engineering unit '", old_name, "' has an unrecognized type and cannot be renamed.");

  addEngUnit(std::move(unit_ptr));
}

bool
CSGEngUnitList::operator==(const CSGEngUnitList & other) const
{
  const auto all_units = this->getAllEngUnits();
  const auto other_units = other.getAllEngUnits();

  // Check that same number of engineering units are defined in both lists
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
