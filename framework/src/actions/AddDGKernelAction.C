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

#include "AddDGKernelAction.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddDGKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddDGKernelAction::AddDGKernelAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddDGKernelAction::act()
{
  _problem->addDGKernel(_type, getShortName(), _moose_object_pars);
}
