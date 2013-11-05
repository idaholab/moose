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

#ifndef SETUPPREDICTORACTION_H
#define SETUPPREDICTORACTION_H

#include "MooseObjectAction.h"

class SetupPredictorAction;

template<>
InputParameters validParams<SetupPredictorAction>();

/**
 * Sets the predictor
 */
class SetupPredictorAction : public MooseObjectAction
{
public:
  SetupPredictorAction(const std::string & name, InputParameters parameters);
  virtual ~SetupPredictorAction();

  virtual void act();

protected:

};


#endif /* SETUPPREDICTORACTION_H_ */
