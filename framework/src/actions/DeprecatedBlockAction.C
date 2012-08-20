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

#include "DeprecatedBlockAction.h"

template<>
InputParameters validParams<DeprecatedBlockAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<bool>("DEPRECATED", "*** WARNING: This block is deprecated - DO NOT USE ***");
  return params;
}

DeprecatedBlockAction::DeprecatedBlockAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters)
{
}

DeprecatedBlockAction::~DeprecatedBlockAction()
{
}

void
DeprecatedBlockAction::act()
{
  mooseError("Input file block '" + name()  + "' has been deprecated.");
}
