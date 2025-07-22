//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

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
  MooseEnum numeric_types("real complex", "real");
  params.addParam<MooseEnum>(
      "numeric_type", numeric_types, "Number type used for the problem. Can be real or complex.");

  return params;
}

MFEMExecutioner::MFEMExecutioner(const InputParameters & parameters)
  : Executioner(parameters),
    _mfem_problem(dynamic_cast<MFEMProblem &>(feProblem())),
    _problem_data(_mfem_problem.getProblemData())
{
  setDevice();
  setNumericType();
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

void
MFEMExecutioner::setNumericType()
{
  MooseEnum numeric_type = getParam<MooseEnum>("numeric_type");
  if (numeric_type == "real")
    _problem_data.num_type = MFEMProblemData::NumericType::REAL;
  else if (numeric_type == "complex")
    _problem_data.num_type = MFEMProblemData::NumericType::COMPLEX;
  else
    mooseError("Unknown numeric type. Please set the numeric type to either 'real' or 'complex'.");
}

#endif
