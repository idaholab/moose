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

#ifndef CONSTANTSCALARAUX_H
#define CONSTANTSCALARAUX_H

#include "AuxScalarKernel.h"

class ConstantScalarAux;

template<>
InputParameters validParams<ConstantScalarAux>();

/**
 * Sets a constant value on a scalar variable
 */
class ConstantScalarAux : public AuxScalarKernel
{
public:
  ConstantScalarAux(const std::string & name, InputParameters parameters);
  virtual ~ConstantScalarAux();

protected:
  virtual Real computeValue();

  const Real & _value;
};


#endif /* CONSTANTSCALARAUX_H */
