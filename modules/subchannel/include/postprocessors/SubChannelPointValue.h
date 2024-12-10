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
 * Calculates a user selected variable at a user selected point in the assembly
 */
class SubChannelPointValue : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  SubChannelPointValue(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() const override;

protected:
  SubChannelMesh & _mesh;
  const Real & _height;
  const int & _i_ch;
  Point _point;
  const unsigned int _var_number;
  const System & _system;
  Real _value;
};
