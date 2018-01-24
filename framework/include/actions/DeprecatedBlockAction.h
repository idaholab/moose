//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DEPRECATEDBLOCKACTION_H
#define DEPRECATEDBLOCKACTION_H

#include "Action.h"

class DeprecatedBlockAction;

template <>
InputParameters validParams<DeprecatedBlockAction>();

/**
 * Used for marking that some block are deprecated and not be used
 */
class DeprecatedBlockAction : public Action
{
public:
  DeprecatedBlockAction(InputParameters parameters);

  void act() override;
};

#endif // DEPRECATEDBLOCKACTION_H
