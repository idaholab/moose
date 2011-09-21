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

#include "AddKernelAction.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddKernelAction::AddKernelAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddKernelAction::act()
{
  is_kernels_action = Parser::pathContains(_name, "Kernels");

  if (is_kernels_action)
    _problem->addKernel(_type, getShortName(), _moose_object_pars);
  else
    _problem->addAuxKernel(_type, getShortName(), _moose_object_pars);
}
