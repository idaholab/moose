//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

class CohesiveZoneActionBase : public Action
{
public:
  static InputParameters validParams();

  CohesiveZoneActionBase(const InputParameters & params);

  ///@{ output methods
  static MultiMooseEnum outputPropertiesType();
  static MultiMooseEnum materialOutputOrders();
  static MultiMooseEnum materialOutputFamilies();
  ///@}

  ///@{ table data for output generation
  static const std::map<std::string, std::string> _real_vector_cartesian_component_table;
  static const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
      _vector_direction_table;
  static const std::vector<char> _component_table;
  ///@}
};
