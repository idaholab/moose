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

#ifndef NODALMAXVALUE_H
#define NODALMAXVALUE_H

#include "NodalPostprocessor.h"

class MooseVariable;

//Forward Declarations
class NodalMaxValue;

template<>
InputParameters validParams<NodalMaxValue>();

class NodalMaxValue : public NodalPostprocessor
{
public:
  NodalMaxValue(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  Real _value;
};

#endif
