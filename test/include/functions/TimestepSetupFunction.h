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

#ifndef TIMESTEPSETUPFUNCTION_H
#define TIMESTEPSETUPFUNCTION_H

#include "Function.h"

class TimestepSetupFunction;

template<>
InputParameters validParams<TimestepSetupFunction>();

class TimestepSetupFunction : public Function
{
public:
  TimestepSetupFunction(const std::string & name, InputParameters parameters);

  virtual Real value(Real t, const Point & p);

  virtual void timestepSetup();
private:
  unsigned int &  _local_timestep;
};

#endif //TIMESTEPSETUPFUNCTION_H
