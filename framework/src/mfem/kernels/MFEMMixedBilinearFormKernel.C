//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedBilinearFormKernel.h"

InputParameters
MFEMMixedBilinearFormKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Base class for mixed bilinear form kernels, allowing different trial and test variables.");
  params.addParam<VariableName>(
      "trial_variable",
      "The trial variable this kernel is acting on and which will be solved for. If empty "
      "(default), it will be the same as the test variable.");
  return params;
}

MFEMMixedBilinearFormKernel::MFEMMixedBilinearFormKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _trial_var_name(isParamValid("trial_variable") ? getParam<VariableName>("trial_variable")
                                                   : _test_var_name)
{
}

const VariableName &
MFEMMixedBilinearFormKernel::getTrialVariableName() const
{
  return _trial_var_name;
}

#endif
