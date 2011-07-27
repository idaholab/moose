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

#ifndef ADDFUNCTIONACTION_H
#define ADDFUNCTIONACTION_H

#include "MooseObjectAction.h"

class AddFunctionAction;

template<>
InputParameters validParams<AddFunctionAction>();


/**
 * This class parses functions in the [Functions] block and creates them.
 */
class AddFunctionAction : public MooseObjectAction
{
public:
  AddFunctionAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif //ADDFUNCTIONACTION_H
