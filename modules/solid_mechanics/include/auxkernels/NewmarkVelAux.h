/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
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
