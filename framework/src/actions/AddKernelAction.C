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
#include "FEProblem.h"

template <>
InputParameters
validParams<AddKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddKernelAction::AddKernelAction(InputParameters params) : MooseObjectAction(params) {}

void
AddKernelAction::act()
{
  if (_current_task == "add_kernel")
    _problem->addKernel(_type, _name, _moose_object_pars);
  else
  {
    if (getAllTasks().find("add_aux_bc") != getAllTasks().end())
      mooseWarning("The [AuxBCs] block is deprecated, all AuxKernels including both block and "
                   "boundary restricted should be added within the [AuxKernels] block");

    _problem->addAuxKernel(_type, _name, _moose_object_pars);
  }
}
