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

#ifndef NEWMARKACCELAUX_H
#define NEWMARKACCELAUX_H

#include "AuxKernel.h"


//Forward Declarations
class NewmarkAccelAux;

template<>
InputParameters validParams<NewmarkAccelAux>();

/**
 * Accumulate values from one auxiliary variable into another
 */
class NewmarkAccelAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NewmarkAccelAux(const std::string & name, InputParameters parameters);

  virtual ~NewmarkAccelAux() {}

protected:
  virtual Real computeValue();

  VariableValue & _disp_old;
  VariableValue & _disp;
  VariableValue & _vel_old;
  Real _beta;

};

#endif //NewmarkAccelAux_H
