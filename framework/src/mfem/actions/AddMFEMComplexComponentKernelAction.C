//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "AddMFEMComplexComponentKernelAction.h"

registerMooseAction("MooseApp", AddMFEMComplexComponentKernelAction, "add_mfem_complex_component_kernels");

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
AddMFEMComplexComponentKernelAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an MFEM AuxKernel to serve as the real or imaginary component of an MFEMComplexKernel.");
  return params;
}

AddMFEMComplexComponentKernelAction::AddMFEMComplexComponentKernelAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMComplexComponentKernelAction::act()
{
  std::string action_name = _app.actionWarehouse().getCurrentActionName();
  if (!(_name == "real_part" || _name == "imag_part"))
    mooseError("The name of the complex component kernel must be either 'real_part' or 'imag_part'. "
               "Current name is: " + _name + ". ");
  
  int second_last_slash = nthFromLast(action_name, '/', 2);
  if (second_last_slash == -1)
    mooseError("Could not find the parent MFEMComplexKernel name in the action name: " + action_name + ". "
      "Please ensure that your script contains a block named ComplexKernels and that each of your "
      "MFEMComplexKernels within it has a real_part and an imag_part sub-block.");
  
  std::string comp_name = action_name.substr(second_last_slash + 1, action_name.length());
  std::cout << "Component name = " << comp_name << std::endl;


  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addAuxKernel(_type, comp_name, _moose_object_pars);
}

#endif
