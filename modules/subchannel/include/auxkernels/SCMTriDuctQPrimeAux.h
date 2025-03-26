//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiffusionFluxAux.h"

/**
 * Computes linear heat rate to/from the hexagonal duct's inner surface to subchannels.
 */
class SCMTriDuctQPrimeAux : public DiffusionFluxAux
{
public:
  static InputParameters validParams();

  SCMTriDuctQPrimeAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// flat-to-flat distance
  const Real & _flat_to_flat;
};
