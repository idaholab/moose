//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "SubChannelMesh.h"

/**
 * Calculates an overall Delta of a chosen variable for the subchannel assembly
 */
class SubChannelDelta : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  SubChannelDelta(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() const override;

protected:
  SubChannelMesh & _mesh;
  AuxVariableName const & _variable;
  Real _value;
};
