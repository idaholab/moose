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

#ifndef OBJECTACTION_H
#define OBJECTACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class ObjectAction;

template<>
InputParameters validParams<ObjectAction>();


class ObjectAction : public Action
{
public:
  ObjectAction(const std::string & name, InputParameters params);

  virtual InputParameters & getObjectParams() = 0;

protected:
};

#endif // OBJECTACTION_H
