//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
