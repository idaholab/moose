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
  InputParameters params = emptyInputParameters();
  params.addClassDescription("Executioner for MFEM problems.");
  params.addParam<std::string>("device", "cpu", "Run app on the chosen device.");
  MooseEnum assembly_levels("legacy full element partial none", "legacy", true);
  params.addParam<MooseEnum>(
      "assembly_level",
      assembly_levels,
      "Matrix assembly level. Options: legacy, full, element, partial, none.");
  return params;
}

MFEMExecutioner::MFEMExecutioner(const InputParameters & params, MFEMProblem & mfem_problem)
  : _mfem_problem(mfem_problem), _problem_data(_mfem_problem.getProblemData())
{
  setDevice(params.get<std::string>("device"));
}

void
MFEMExecutioner::setDevice(const std::string & device_name)
{
  // TODO: might not be enough should check the device
  // your trying to donfigure is the same one that has been configured
  if (_device.IsConfigured())
    return;
  _device.Configure(device_name);
  _device.Print(Moose::out);
}

#endif
