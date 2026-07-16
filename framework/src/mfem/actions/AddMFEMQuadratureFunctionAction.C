//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMQuadratureFunctionAction.h"

registerMooseAction("MooseApp", AddMFEMQuadratureFunctionAction, "add_mfem_quadrature_functions");

InputParameters
AddMFEMQuadratureFunctionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an MFEM quadrature function coefficient to the simulation.");
  return params;
}

AddMFEMQuadratureFunctionAction::AddMFEMQuadratureFunctionAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMQuadratureFunctionAction::act()
{
  if (_problem->feBackend() == Moose::FEBackend::MFEM)
    static_cast<MFEMProblem &>(*_problem).addQuadratureFunction(_type, _name, _moose_object_pars);
}

#endif
