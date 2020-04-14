//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

/**
 * This postprocessor computes the mass by integrating the density over the volume.
 */
template <bool is_ad>
class MassTempl : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  MassTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();
  const GenericMaterialProperty<Real, is_ad> & _density;
};

typedef MassTempl<false> Mass;
typedef MassTempl<true> ADMass;
