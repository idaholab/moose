//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChainControlDataSystem.h"

ChainControlDataSystem::ChainControlDataSystem(MooseApp & app) : _app(app) {}

bool
ChainControlDataSystem::hasChainControlData(const std::string & data_name) const
{
  return _chain_control_data_map.find(data_name) != _chain_control_data_map.end();
}

void
ChainControlDataSystem::copyValuesBack()
{
  for (const auto & item : _chain_control_data_map)
    item.second->copyValuesBack();
}

const std::map<std::string, std::unique_ptr<ChainControlDataBase>> &
ChainControlDataSystem::getChainControlDataMap() const
{
  return _chain_control_data_map;
}

std::string
ChainControlDataSystem::outputChainControlMap() const
{
  std::string map_str = "";
  for (const auto & item : _chain_control_data_map)
  {
    map_str += item.first + " (" + item.second->type() + ") declared? " +
               (item.second->getDeclared() ? "true" : "false");
    if (const auto ctl_real = dynamic_cast<ChainControlData<Real> *>(item.second.get()))
      map_str += ". Current value: " + std::to_string(ctl_real->get());
    if (const auto ctl_bool = dynamic_cast<ChainControlData<bool> *>(item.second.get()))
      map_str += ". Current value: " + std::to_string(ctl_bool->get());
    map_str += "\n";
  }

  return map_str;
}
