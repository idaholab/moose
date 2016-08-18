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

#include "AddArrayKernelAction.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddArrayKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddArrayKernelAction::AddArrayKernelAction(InputParameters params) :
    MooseObjectAction(params)
{
}

void
AddArrayKernelAction::act()
{
  if (_current_task == "add_array_kernel")
    _problem->addKernel(_type, _name, _moose_object_pars);
//  else
//    _problem->addAuxArrayKernel(_type, _name, _moose_object_pars);
}
