/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef VALUETHRESHOLDMARKER_H
#define VALUETHRESHOLDMARKER_H

#include "QuadraturePointMarker.h"

class ValueThresholdMarker;

template <>
InputParameters validParams<ValueThresholdMarker>();

class ValueThresholdMarker : public QuadraturePointMarker
{
public:
  ValueThresholdMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeQpMarker() override;

  bool _coarsen_set;
  Real _coarsen;
  bool _refine_set;
  Real _refine;

  bool _invert;
  MarkerValue _third_state;

  const VariableValue & _u;
};

#endif /* VALUETHRESHOLDMARKER_H */
