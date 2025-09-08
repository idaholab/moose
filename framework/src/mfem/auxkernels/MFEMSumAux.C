//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSumAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMSumAux);

InputParameters
MFEMSumAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the sum of two variables sharing an FE space, each optionally scaled by a real "
      "constant, and stores the result in a third.");
  params.addRequiredParam<VariableName>("first_source_variable", "First variable to sum.");
  params.addRequiredParam<VariableName>("second_source_variable", "Second variable to sum.");
  params.addParam<mfem::real_t>(
      "first_scale_factor", 1.0, "Factor to scale the first variable by prior to sum.");
  params.addParam<mfem::real_t>(
      "second_scale_factor", 1.0, "Factor to scale the second variable by prior to sum.");
  return params;
}

MFEMSumAux::MFEMSumAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _v1_var_name(getParam<VariableName>("first_source_variable")),
    _v2_var_name(getParam<VariableName>("second_source_variable")),
    _v1_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_v1_var_name)),
    _v2_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_v2_var_name)),
    _lambda1(getParam<mfem::real_t>("first_scale_factor")),
    _lambda2(getParam<mfem::real_t>("second_scale_factor"))
{
}

void
MFEMSumAux::execute()
{
  // result = lambda1 * v1 + lambda2 * v2
  add(_lambda1, _v1_var, _lambda2, _v2_var, _result_var);
}

#endif
