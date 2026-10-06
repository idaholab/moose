//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTimeDerivativeVectorMassKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMTimeDerivativeVectorMassKernel);

InputParameters
MFEMTimeDerivativeVectorMassKernel::validParams()
{
  InputParameters params = MFEMVectorMassKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form "
      "$(k \\dot{\\vec u}, \\vec v)_\\Omega$, where $\\vec u$ and $\\vec v$ are in "
      "vector $H^1$ or $L^2$ spaces.");
  return params;
}

MFEMTimeDerivativeVectorMassKernel::MFEMTimeDerivativeVectorMassKernel(
    const InputParameters & parameters)
  : MFEMVectorMassKernel(parameters),
    _var_dot_name(getMFEMProblem().getProblemData().time_derivative_map.getTimeDerivativeName(
        _trial_var_name))
{
}

#endif
