//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "UserObject.h"
#include "ScalarCoupleable.h"
#include "MortarConsumerInterface.h"
#include "TwoMaterialPropertyInterface.h"
#include "NeighborCoupleable.h"

/**
 * Base class for creating new nodally-based mortar user objects
 */
class MortarUserObject : public UserObject,
                         public ScalarCoupleable,
                         public MortarConsumerInterface,
                         public TwoMaterialPropertyInterface,
                         public NeighborCoupleable
{
public:
  static InputParameters validParams();

  MortarUserObject(const InputParameters & parameters);

  /**
   * reinitialize any data relevant to the current mortar segment
   */
  virtual void reinit() = 0;
  virtual void threadJoin(const UserObject &) override final {}
};
