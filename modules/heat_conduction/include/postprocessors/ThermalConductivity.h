/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef THERMALCONDUCTIVITY_H
#define THERMALCONDUCTIVITY_H

#include "SideAverageValue.h"

//Forward Declarations
class ThermalConductivity;

template<>
InputParameters validParams<ThermalConductivity>();

/**
 * This postprocessor computes the thermal conductivity of the bulk.
 */
class ThermalConductivity : public SideAverageValue
{
public:
  ThermalConductivity(const InputParameters & parameters);

  virtual Real getValue();

protected:
  const Real _dx;
  const PostprocessorValue & _flux;
  const PostprocessorValue & _T_hot;
  const Real _length_scale;
  const Real _k0;
};

#endif //THERMALCONDUCTIVITY_H
