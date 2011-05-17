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

#ifndef DONOTHINGAUX_H_
#define DONOTHINGAUX_H_

#include "AuxKernel.h"

class DoNothingAux;

template<>
InputParameters validParams<DoNothingAux>();

/**
 * Do-nothing aux kernel
 */
class DoNothingAux : public AuxKernel
{
public:
  DoNothingAux(const std::string & name, InputParameters parameters);
  virtual ~DoNothingAux();

protected:

  virtual Real computeValue();

};



#endif /* DONOTHINGAUX_H_ */
