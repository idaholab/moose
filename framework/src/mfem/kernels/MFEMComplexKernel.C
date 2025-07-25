//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMComplexKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMComplexKernel);

InputParameters
MFEMComplexKernel::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += MFEMBlockRestrictable::validParams();
  params.registerBase("Kernel");
  params.addParam<VariableName>("variable",
                                "Variable labelling the weak form this kernel is added to");
  params.addClassDescription("Holds MFEMKernel objects for the real and imaginary parts of a complex kernel.");
  //params.addParam<UserObjectName>(
  //    "real_kernel", "Name of the kernel to use for the real of the composite complex kernel.");
  //params.addParam<UserObjectName>(
  //    "imag_kernel", "Name of the kernel to use for the imaginary of the composite complex kernel.");

  return params;
}

MFEMComplexKernel::MFEMComplexKernel(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    MFEMBlockRestrictable(parameters, getMFEMProblem().mesh().getMFEMParMesh()),
   _test_var_name(getParam<VariableName>("variable"))
{
}

#endif
