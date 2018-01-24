//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MASS_H
#define MASS_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class Mass;

template <>
InputParameters validParams<Mass>();

/**
 * This postprocessor computes the mass by integrating the density over the volume.
 */

class Mass : public ElementIntegralVariablePostprocessor
{
public:
  Mass(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();
  const MaterialProperty<Real> & _density;
};

#endif // MASS_H
