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

#ifndef SETUPEIGENACTION_H
#define SETUPEIGENACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class SetupEigenAction;

template<>
InputParameters validParams<SetupEigenAction>();

/**
 *
 */
class SetupEigenAction : public Action
{
public:

  /**
   * Class constructor
   * @param name
   */
  SetupEigenAction(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~SetupEigenAction(){};

  /**
   *
   */
  virtual void act();
};

#endif //SETUPEIGENACTION_H
