//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSERVEDLANGEVINNOISE_H
#define CONSERVEDLANGEVINNOISE_H

#include "LangevinNoise.h"
#include "ConservedNoiseBase.h"

// Forward Declarations
class ConservedLangevinNoise;

template <>
InputParameters validParams<ConservedLangevinNoise>();

class ConservedLangevinNoise : public LangevinNoise
{
public:
  ConservedLangevinNoise(const InputParameters & parameters);

protected:
  virtual void residualSetup(){};
  virtual Real computeQpResidual();

private:
  const ConservedNoiseInterface & _noise;
};

#endif // CONSERVEDLANGEVINNOISE_H
