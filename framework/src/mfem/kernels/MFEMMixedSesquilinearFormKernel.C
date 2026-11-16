//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedSesquilinearFormKernel.h"

registerMooseObject("MooseApp", MFEMMixedSesquilinearFormKernel);

InputParameters
MFEMMixedSesquilinearFormKernel::validParams()
{
  InputParameters params = MFEMComplexKernel::validParams();
  params.addClassDescription(
      "Base class for mixed sesquilinear form kernels, allowing different trial and test "
      "variables. Transposition of each component is controlled individually via the `transpose` "
      "parameter on each `RealComponent`/`ImagComponent` sub-block.");
  params.addParam<VariableName>(
      "trial_variable",
      "The variable coupled to the test variable by this kernel. It may be eliminated and need not "
      "be solved for by this weak form's equation system. Defaults to the test variable.");
  params.renameParam("trial_variable", "coupled_variable", "");
  return params;
}

MFEMMixedSesquilinearFormKernel::MFEMMixedSesquilinearFormKernel(const InputParameters & parameters)
  : MFEMComplexKernel(parameters),
    _trial_var_name(isParamValid("trial_variable") ? getParam<VariableName>("trial_variable")
                                                   : _test_var_name)
{
}

const VariableName &
MFEMMixedSesquilinearFormKernel::getTrialVariableName() const
{
  return _trial_var_name;
}

#endif
