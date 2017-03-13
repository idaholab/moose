/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
