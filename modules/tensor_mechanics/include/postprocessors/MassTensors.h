/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MASSTENSORS_H
#define MASSTENSORS_H

#include "ElementIntegralVariablePostprocessor.h"

//Forward Declarations
class MassTensors;

template<>
InputParameters validParams<MassTensors>();

/**
 * This postprocessor computes the mass by integrating the density over the volume
 * and is a replicate of the mass postprocessor in solid mechanics; the replicate is
 * necessary for the migration to a single continuum mechanics module.
 */
class MassTensors: public ElementIntegralVariablePostprocessor
{
public:
  MassTensors(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();
  const MaterialProperty<Real> & _density;
};

#endif //MASSTENSORS_H
