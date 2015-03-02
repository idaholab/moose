/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef THERMALCOND_H
#define THERMALCOND_H

#include "SideAverageValue.h"

//Forward Declarations
class ThermalCond;

template<>
InputParameters validParams<ThermalCond>();

/**
 * This postprocessor computes the thermal conductivity of the bulk.
 */
class ThermalCond : public SideAverageValue
{
public:
  ThermalCond(const std::string & name, InputParameters parameters);

  virtual Real getValue();

  Real _dx;
  Real _flux;
  Real _T_hot;
  Real _length_scale;
  Real _k0;

private:
};

#endif
