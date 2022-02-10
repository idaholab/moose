//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVolumeJunctionBaseUserObject.h"
#include "ADShaftConnectableUserObjectInterface.h"

/**
 * Test component for showing how to connect a junction-derived object to a shaft
 */
class ADShaftConnectedTestComponentUserObject : public ADVolumeJunctionBaseUserObject,
                                                public ADShaftConnectableUserObjectInterface
{
public:
  ADShaftConnectedTestComponentUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  const ADVariableValue & _rhoA;
  const ADVariableValue & _rhouA;
  const ADVariableValue & _rhoEA;
  const ADVariableValue & _jct_var;
  const ADVariableValue & _omega;

public:
  static InputParameters validParams();
};
