//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

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
