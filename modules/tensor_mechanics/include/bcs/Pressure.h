/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PRESSURE_H
#define PRESSURE_H

#include "IntegratedBC.h"

class Function;
class Pressure;

template <>
InputParameters validParams<Pressure>();

/**
 * Pressure applies a pressure on a given boundary in the direction defined by component
 */
class Pressure : public IntegratedBC
{
public:
  Pressure(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const int _component;

  const Real _factor;

  Function * const _function;

  const PostprocessorValue * const _postprocessor;

  /// _alpha Parameter for HHT time integration scheme
  const Real _alpha;
};

#endif // PRESSURE_H
