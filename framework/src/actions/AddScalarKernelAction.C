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

#include "AddScalarKernelAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddScalarKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddScalarKernelAction::AddScalarKernelAction(InputParameters params) : MooseObjectAction(params) {}

void
AddScalarKernelAction::act()
{
  if (_current_task == "add_scalar_kernel")
    _problem->addScalarKernel(_type, _name, _moose_object_pars);
  else
    _problem->addAuxScalarKernel(_type, _name, _moose_object_pars);
}
