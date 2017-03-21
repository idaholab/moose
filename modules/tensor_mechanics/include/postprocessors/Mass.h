/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
