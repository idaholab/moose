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

#ifndef INITIALREFINEMENTACTION_H
#define INITIALREFINEMENTACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

//Forward Declaration
class MooseMesh;

class InitialRefinementAction;

template<>
InputParameters validParams<InitialRefinementAction>();


class InitialRefinementAction : public Action
{
public:
  InitialRefinementAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif // INITIALREFINEMENTACTION_H
