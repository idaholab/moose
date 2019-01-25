//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddADKernelAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddADKernelAction, "add_ad_kernel");

template <>
InputParameters
validParams<AddADKernelAction>()
{
  InputParameters params = validParams<MooseADObjectAction>();
  params.addClassDescription(
      "This action is used to add ADKernel<RESIDUAL> and ADKernel<JACOBIAN> objects");
  return params;
}

AddADKernelAction::AddADKernelAction(InputParameters params) : MooseADObjectAction(params) {}

void
AddADKernelAction::act()
{
  flagDoingAD();
  _problem->addKernel(_base_type + "<RESIDUAL>", _name + "_residual", _moose_object_pars);
  _problem->addKernel(_base_type + "<JACOBIAN>", _name + "_jacobian", _moose_object_pars);
}
