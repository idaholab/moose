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

#ifndef NEWMARKVELAUX_H
#define NEWMARKVELAUX_H

#include "AuxKernel.h"


//Forward Declarations
class NewmarkVelAux;

template<>
InputParameters validParams<NewmarkVelAux>();

/**
 * Accumulate values from one auxiliary variable into another
 */
class NewmarkVelAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NewmarkVelAux(const std::string & name, InputParameters parameters);

  virtual ~NewmarkVelAux() {}

protected:
  virtual Real computeValue();

  VariableValue & _accel_old;
  VariableValue & _accel;
  Real _gamma;

};

#endif //NewmarkVelAux_H
