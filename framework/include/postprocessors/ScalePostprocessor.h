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
#ifndef SCALEPOSTPROCESSOR_H
#define SCALEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class ScalePostprocessor;

template <>
InputParameters validParams<ScalePostprocessor>();

/**
 * Scale a postprocessor
 */
class ScalePostprocessor : public GeneralPostprocessor
{
public:
  ScalePostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual PostprocessorValue getValue() override;

protected:
  const PostprocessorValue & _value;
  Real _scaling_factor;
};

#endif /* SCALEPOSTPROCESSOR_H */
