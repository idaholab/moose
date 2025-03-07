#ifdef MFEM_ENABLED

#pragma once
#include "MFEMAuxKernel.h"
#include "MFEMProblem.h"

InputParameters
MFEMAuxKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("AuxKernel");
  params.addClassDescription("Base class for MFEMGeneralUserObjects that update auxiliary "
                             "variables outside of the main solve step.");
  params.addRequiredParam<AuxVariableName>("variable",
                                           "The name of the variable that this object applies to");
  return params;
}

MFEMAuxKernel::MFEMAuxKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _result_var_name(getParam<AuxVariableName>("variable")),
    _result_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_result_var_name))
{
  _result_var = 0.0;
}

#endif
