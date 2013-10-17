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

#ifndef RANDOMAUX_H
#define RANDOMAUX_H

#include "AuxKernel.h"

//Forward Declarations
class RandomAux;
class RandomElementalUserObject;

template<>
InputParameters validParams<RandomAux>();

/**
 * An AuxKernel that uses built-in Random number generation.
 */
class RandomAux : public AuxKernel
{
public:
  RandomAux(const std::string & name, InputParameters params);

  virtual ~RandomAux();

protected:
  virtual Real computeValue();

  const RandomElementalUserObject *_random_uo;
  bool _generate_ints;
};

#endif //RANDOMAUX_H
