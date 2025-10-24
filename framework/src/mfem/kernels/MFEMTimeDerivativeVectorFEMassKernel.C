//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTimeDerivativeVectorFEMassKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMTimeDerivativeVectorFEMassKernel);

InputParameters
MFEMTimeDerivativeVectorFEMassKernel::validParams()
{
  InputParameters params = MFEMVectorFEMassKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k \\dot{\\vec u}, \\vec v)_\\Omega$ "
                             "arising from the weak form of the operator "
                             "$k \\dot{\\vec u}$.");
  return params;
}

MFEMTimeDerivativeVectorFEMassKernel::MFEMTimeDerivativeVectorFEMassKernel(
    const InputParameters & parameters)
  : MFEMVectorFEMassKernel(parameters),
    _var_dot_name(
        getMFEMProblem().getProblemData().time_derivative_map.getTimeDerivativeName(_test_var_name))
{
}

#endif
