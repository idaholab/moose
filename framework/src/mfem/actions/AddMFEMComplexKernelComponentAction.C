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
  MooseUtils::stringSplit(_pars.blockFullpath(), elements, 1, "/");

  if (_problem->feBackend() == Moose::FEBackend::MFEM && _name == "RealComponent")
    static_cast<MFEMProblem &>(*_problem).addRealComponentToKernel(
        _type, elements[elements.size() - 2], _moose_object_pars);
  else if (_problem->feBackend() == Moose::FEBackend::MFEM && _name == "ImagComponent")
    static_cast<MFEMProblem &>(*_problem).addImagComponentToKernel(
        _type, elements[elements.size() - 2], _moose_object_pars);
}

#endif
