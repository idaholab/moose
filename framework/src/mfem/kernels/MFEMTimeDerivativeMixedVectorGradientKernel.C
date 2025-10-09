//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTimeDerivativeMixedVectorGradientKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMTimeDerivativeMixedVectorGradientKernel);

InputParameters
MFEMTimeDerivativeMixedVectorGradientKernel::validParams()
{
  InputParameters params = MFEMMixedVectorGradientKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k\\vec\\nabla du_dt, \\vec v)_\\Omega$ "
      "arising from the weak form of the gradient operator "
      "$k\\vec \\nabla du/dt$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMTimeDerivativeMixedVectorGradientKernel::MFEMTimeDerivativeMixedVectorGradientKernel(
    const InputParameters & parameters)
  : MFEMMixedVectorGradientKernel(parameters),
    _var_dot_name(getMFEMProblem().getProblemData().time_derivative_map.getTimeDerivativeName(
        _trial_var_name))
{
}

#endif
