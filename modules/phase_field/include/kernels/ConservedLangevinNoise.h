//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LangevinNoise.h"
#include "ConservedNoiseBase.h"

class ConservedLangevinNoise : public LangevinNoise
{
public:
  static InputParameters validParams();

  ConservedLangevinNoise(const InputParameters & parameters);

protected:
  virtual void residualSetup(){};
  virtual Real computeQpResidual();

private:
  const ConservedNoiseInterface & _noise;
};
