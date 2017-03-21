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

#ifndef RANDOMPOSTPROCESSOR_H
#define RANDOMPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"
#include "MooseRandom.h"

// Forward Declarations
class RandomPostprocessor;

template <>
InputParameters validParams<RandomPostprocessor>();

/**
 * Just returns a random number.
 */
class RandomPostprocessor : public GeneralPostprocessor
{
public:
  RandomPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;

private:
  const unsigned int _generator_id;

  MooseRandom & _random;
};

#endif // RANDOMPOSTPROCESSOR_H
