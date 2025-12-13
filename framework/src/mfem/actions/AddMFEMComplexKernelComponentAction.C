//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "AddMFEMComplexKernelComponentAction.h"

registerMooseAction("MooseApp",
                    AddMFEMComplexKernelComponentAction,
                    "add_mfem_complex_kernel_components");

InputParameters
AddMFEMComplexKernelComponentAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Add an MFEMKernel to serve as the real or imaginary component of an MFEMComplexKernel.");
  return params;
}

AddMFEMComplexKernelComponentAction::AddMFEMComplexKernelComponentAction(
    const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMComplexKernelComponentAction::act()
{
  std::vector<std::string> elements;
  MooseUtils::tokenize<std::string>(_pars.blockFullpath(), elements);
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());

  if (mfem_problem && _name == "RealComponent")
    mfem_problem->addRealComponentToKernel(
        _type, elements[elements.size() - 2], _moose_object_pars);
  else if (mfem_problem && _name == "ImagComponent")
    mfem_problem->addImagComponentToKernel(
        _type, elements[elements.size() - 2], _moose_object_pars);
}

#endif
