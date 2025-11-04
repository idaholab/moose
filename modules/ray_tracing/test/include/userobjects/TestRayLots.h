//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LotsOfRaysRayStudy.h"

class TestRayLots : public LotsOfRaysRayStudy
{
public:
  TestRayLots(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  void postExecuteStudy() override final;
};
