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

class NodalL2Error;

template <>
InputParameters validParams<NodalL2Error>();

/**
 *
 */
class NodalL2Error : public NodalVariablePostprocessor
{
public:
  NodalL2Error(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _integral_value;
  Function & _func;
};

#endif /* NODALL2ERROR_H */
