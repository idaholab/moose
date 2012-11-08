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

#ifndef SELFAUX_H
#define SELFAUX_H

#include "AuxKernel.h"


//Forward Declarations
class SelfAux;

template<>
InputParameters validParams<SelfAux>();

/**
 * Self auxiliary value
 */
class SelfAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  SelfAux(const std::string & name, InputParameters parameters);

  virtual ~SelfAux();

protected:
  virtual Real computeValue();
};

#endif //SELFAUX_H
