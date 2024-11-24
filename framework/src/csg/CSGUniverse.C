//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGUniverse.h"

namespace CSG
{

CSGUniverse::CSGUniverse(const std::string name) : _name(name) { _next_cell_id = 0; }

std::shared_ptr<CSGCell>
CSGUniverse::addMaterialCell(const std::string name, const std::string fill_name)
{
  if (_cell_name_id_mapping.find(name) != _cell_name_id_mapping.end())
    mooseError("Cell with name " + name + " already exists in universe");
  const auto cell_id = _next_cell_id++;
  _cells.insert(std::make_pair(cell_id, std::make_shared<CSGMaterialCell>(name, fill_name)));
  _cell_name_id_mapping.insert({name, cell_id});
  return _cells[cell_id];
}

std::shared_ptr<CSGCell>
CSGUniverse::addVoidCell(const std::string name)
{
  if (_cell_name_id_mapping.find(name) != _cell_name_id_mapping.end())
    mooseError("Cell with name " + name + " already exists in universe");
  const auto cell_id = _next_cell_id++;
  _cells.insert(std::make_pair(cell_id, std::make_shared<CSGVoidCell>(name)));
  _cell_name_id_mapping.insert({name, cell_id});
  return _cells[cell_id];
}

std::shared_ptr<CSGCell>
CSGUniverse::getCell(const std::string name)
{
  if (_cell_name_id_mapping.find(name) == _cell_name_id_mapping.end())
    mooseError("Cell with name " + name + " does not exist in universe");
  const auto cell_id = _cell_name_id_mapping[name];
  return _cells.at(cell_id);
}

bool
CSGUniverse::hasCell(const std::string name) const
{
  return (_cell_name_id_mapping.find(name) != _cell_name_id_mapping.end());
}
} // namespace CSG
