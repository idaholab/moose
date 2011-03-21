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

#ifndef ADAPTIVITYACTION_H_
#define ADAPTIVITYACTION_H_

#include "Action.h"

class AdaptivityAction: public Action
{
public:
  AdaptivityAction(const std::string & name, InputParameters params);

  virtual void act();

  unsigned int getSteps();
};

template<>
InputParameters validParams<AdaptivityAction>();

#endif //ADAPTIVITYACTION_H_
