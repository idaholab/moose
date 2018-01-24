//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TENSORMECHANICSACTIONBASE_H
#define TENSORMECHANICSACTIONBASE_H

#include "Action.h"

class TensorMechanicsActionBase;

template <>
InputParameters validParams<TensorMechanicsActionBase>();

class TensorMechanicsActionBase : public Action
{
public:
  TensorMechanicsActionBase(const InputParameters & params);

  static MultiMooseEnum outputPropertiesType();

public:
  ///@{ table data for output generation
  static const std::map<std::string, std::string> _ranktwoaux_table;
  static const std::vector<char> _component_table;
  static const std::map<std::string, std::pair<std::string, std::vector<std::string>>>
      _ranktwoscalaraux_table;
  ///@}
};

#endif // TENSORMECHANICSACTIONBASE_H
