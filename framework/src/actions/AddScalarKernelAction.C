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

template<>
InputParameters validParams<AddScalarKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddScalarKernelAction::AddScalarKernelAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddScalarKernelAction::act()
{
  if (getAction() == "add_scalar_kernel")
    _problem->addScalarKernel(_type, getShortName(), _moose_object_pars);
  else
    _problem->addAuxScalarKernel(_type, getShortName(), _moose_object_pars);
}
