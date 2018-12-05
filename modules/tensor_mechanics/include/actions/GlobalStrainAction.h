//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GLOBALSTRAINACTION_H
#define GLOBALSTRAINACTION_H

#include "Action.h"

class GlobalStrainAction;

template <>
InputParameters validParams<GlobalStrainAction>();

class GlobalStrainAction : public Action
{
public:
  GlobalStrainAction(const InputParameters & params);

  void act() override;

protected:
  std::vector<VariableName> _disp;
  std::vector<AuxVariableName> _aux_disp;
  std::vector<AuxVariableName> _global_disp;

  std::vector<SubdomainName> _block_names;
  std::set<SubdomainID> _block_ids;
};
#endif // GLOBALSTRAINACTION_H
