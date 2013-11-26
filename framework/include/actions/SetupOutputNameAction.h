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

#ifndef SETUPOUTPUTNAMEACTION_H_
#define SETUPOUTPUTNAMEACTION_H_

#include "Action.h"

class SetupOutputNameAction;

template<>
InputParameters validParams<SetupOutputNameAction>();

/**
 */
class SetupOutputNameAction : public Action
{
public:
  SetupOutputNameAction(const std::string & name, InputParameters parameters);
  virtual ~SetupOutputNameAction();

  virtual void act();

protected:

};


#endif /* SETUPOUTPUTNAMEACTION_H_ */
