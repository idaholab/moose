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

#ifndef ADAPTIVITYACTION_H
#define ADAPTIVITYACTION_H

#include "Action.h"

#ifdef LIBMESH_ENABLE_AMR

class AdaptivityAction;

template<>
InputParameters validParams<AdaptivityAction>();


class AdaptivityAction: public Action
{
public:
  AdaptivityAction(const std::string & name, InputParameters params);

  virtual void act();

  unsigned int getSteps();
};

#endif //LIBMESH_ENABLE_AMR

#endif //ADAPTIVITYACTION_H
