//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMExecutioner.h"
#include "MFEMProblem.h"

InputParameters
MFEMExecutioner::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addClassDescription("Executioner for MFEM problems.");
  params.addParam<std::string>("device", "cpu", "Run app on the chosen device.");
  MooseEnum assembly_levels("legacy full element partial none", "legacy", true);
  params.addParam<MooseEnum>(
      "assembly_level",
      assembly_levels,
      "Matrix assembly level. Options: legacy, full, element, partial, none.");

  return params;
}

MFEMExecutioner::MFEMExecutioner(const InputParameters & parameters)
  : Executioner(parameters),
    _mfem_problem(dynamic_cast<MFEMProblem &>(feProblem())),
    _problem_data(_mfem_problem.getProblemData())
{
  mfem::Hypre::Init();
  setDevice();
}

void
MFEMExecutioner::setDevice()
{
  // TODO: might not be enough should check the device
  // your trying to donfigure is the same one that has been configured
  if (_device.IsConfigured())
    return;
  _device.Configure(getParam<std::string>("device"));
  _device.Print(Moose::out);
}

#endif
