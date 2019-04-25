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
class BreakMeshByBlockManual_3Blocks;

template <>
InputParameters validParams<BreakMeshByBlockManual_3Blocks>();

class BreakMeshByBlockManual_3Blocks : public BreakMeshByBlockManualBase
{
public:
  BreakMeshByBlockManual_3Blocks(const InputParameters & parameters);

  virtual void modify() override;

private:
  void updateElements();
  void addInterfaceBoundary();
};

