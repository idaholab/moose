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

#ifndef ADDFUNCTIONACTION_H_
#define ADDFUNCTIONACTION_H_

#include "MooseObjectAction.h"

/**
 * This class parses functions in the [Functions] block and creates them.
 */
class AddFunctionAction : public MooseObjectAction
{
public:
  AddFunctionAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddFunctionAction>();

#endif //ADDFUNCTIONACTION_H_
