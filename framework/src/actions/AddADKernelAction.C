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
  _problem->addKernel(_type, _name, _moose_object_pars);
  std::string to_erase = "<RESIDUAL>";
  std::string::size_type match = _type.find(to_erase);
  if (match != std::string::npos)
    _type.erase(match, to_erase.length());
  _problem->addKernel(_type + "<JACOBIAN>", _name + "_jacobian", _moose_object_pars);
}
