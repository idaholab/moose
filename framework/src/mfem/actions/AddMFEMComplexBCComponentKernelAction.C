//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "AddMFEMComplexBCComponentKernelAction.h"

registerMooseAction("MooseApp", AddMFEMComplexBCComponentKernelAction, "add_mfem_complex_bc_component_kernels");

int nthFromLast(std::string str, char ch, int N)
{
  int occur = 0;
  for (int i=str.length()-1; i >= 0 ; --i) 
  {
      if (str[i] == ch) {
          occur += 1;
      }
      if (occur == N)
          return i;
  }
  return -1;
}

InputParameters
AddMFEMComplexBCComponentKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an MFEM AuxKernel to serve as the real or imaginary component of an MFEMComplexIntegratedBC.");
  return params;
}

AddMFEMComplexBCComponentKernelAction::AddMFEMComplexBCComponentKernelAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMComplexBCComponentKernelAction::act()
{

  // First, we need to make sure that the parent object is an MFEMComplexIntegratedBC
  std::string action_name = _app.actionWarehouse().getCurrentActionName();
  int second_last_slash = nthFromLast(action_name, '/', 2);

  if (_name == "real_part" || _name == "imag_part")
  {
    std::string comp_name = action_name.substr(second_last_slash + 1, action_name.length());
    MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
    if (mfem_problem)
      mfem_problem->addAuxBoundaryCondition(_type, comp_name, _moose_object_pars);
  }

}

#endif
