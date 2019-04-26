//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BreakMeshByBlockManualBase.h"

// forward declaration
class BreakMeshByBlockManual_2DJunction;

template <>
InputParameters validParams<BreakMeshByBlockManual_2DJunction>();

class BreakMeshByBlockManual_2DJunction : public BreakMeshByBlockManualBase
{
public:
  BreakMeshByBlockManual_2DJunction(const InputParameters & parameters);

  virtual void modify() override;

private:
  void updateElements();
  void addInterfaceBoundary();
};

