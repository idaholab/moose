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

#ifndef MULTIPLEUPDATEAUX_H_
#define MULTIPLEUPDATEAUX_H_

#include "AuxKernel.h"


class MultipleUpdateAux;

template<>
InputParameters validParams<MultipleUpdateAux>();

/**
 * Aux kernel that updated values of coupled variables
 */
class MultipleUpdateAux : public AuxKernel
{
public:
  MultipleUpdateAux(const std::string & name, InputParameters parameters);
  virtual ~MultipleUpdateAux();

protected:
  virtual Real computeValue();

  VariableValue & _nl_u;
  VariableValue & _var1;
  VariableValue & _var2;

};


#endif /* MULTIPLEUPDATEAUX_H_ */
