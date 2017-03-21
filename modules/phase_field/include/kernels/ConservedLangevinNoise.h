/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSERVEDLANGEVINNOISE_H
#define CONSERVEDLANGEVINNOISE_H

#include "LangevinNoise.h"
#include "ConservedNoiseBase.h"

// Forward Declarations
class ConservedLangevinNoise;

template <>
InputParameters validParams<LangevinNoise>();

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
