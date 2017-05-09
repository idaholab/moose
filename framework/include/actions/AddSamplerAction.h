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

#ifndef ADDSAMPLERACTION_H
#define ADDSAMPLERACTION_H

#include "MooseObjectAction.h"

class AddSamplerAction;

template <>
InputParameters validParams<AddSamplerAction>();

/**
 * This class adds a Sampler object.
 * The Sampler contains different sampling strategies and is used to provide
 * random values for sampled parameters using associated distributions. The
 * sampled parameters can be material properties, boundary conditions, initial
 * conditions etc.
 */
class AddSamplerAction : public MooseObjectAction
{
public:
  AddSamplerAction(InputParameters params);

  virtual void act() override;
};

#endif /* ADDSAMPLERACTION_H */
