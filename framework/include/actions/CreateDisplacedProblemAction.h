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

class CreateDisplacedProblemAction;
class SystemBase;

template <>
InputParameters validParams<CreateDisplacedProblemAction>();

/**
 *
 */
class CreateDisplacedProblemAction : public Action
{
public:
  CreateDisplacedProblemAction(InputParameters parameters);

  virtual void act() override;

protected:
  /**
   * Sets up a ProxyRelationshipManager that copies algebraic ghosting from->to
   */
  void addProxyAlgebraicRelationshipManagers(SystemBase & to, SystemBase & from);

  /**
   * Sets up a ProxyRelationshipManager that copies geometric ghosting from->to
   */
  void addProxyGeometricRelationshipManagers(SystemBase & to, SystemBase & from);
};

