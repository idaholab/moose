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

#ifndef NODALL2ERROR_H
#define NODALL2ERROR_H

#include "NodalVariablePostprocessor.h"
#include "FunctionInterface.h"


class NodalL2Error;

template<>
InputParameters validParams<NodalL2Error>();

/**
 *
 */
class NodalL2Error : public NodalVariablePostprocessor,
    public FunctionInterface
{
public:
  NodalL2Error(const std::string & name, InputParameters parameters);
  virtual ~NodalL2Error();

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  Real _integral_value;
  Function & _func;
};

#endif /* NODALL2ERROR_H_ */
