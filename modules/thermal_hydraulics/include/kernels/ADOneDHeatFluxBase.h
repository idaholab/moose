//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class ADHeatFluxFromHeatStructureBaseUserObject;

class ADOneDHeatFluxBase : public ADKernel
{
public:
  ADOneDHeatFluxBase(const InputParameters & parameters);

protected:
  /// User object that computes the heat flux
  const ADHeatFluxFromHeatStructureBaseUserObject & _q_uo;

public:
  static InputParameters validParams();
};
