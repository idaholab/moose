//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTimeDerivativeMassKernel.h"

registerMooseObject("MooseApp", MFEMTimeDerivativeMassKernel);

InputParameters
MFEMTimeDerivativeMassKernel::validParams()
{
  InputParameters params = MFEMMassKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k \\dot{u}, v)_\\Omega$ "
                             "arising from the weak form of the operator "
                             "$k \\dot{u}$.");
  return params;
}

MFEMTimeDerivativeMassKernel::MFEMTimeDerivativeMassKernel(const InputParameters & parameters)
  : MFEMMassKernel(parameters), _var_dot_name(Moose::MFEM::GetTimeDerivativeName(_test_var_name))
{
}

#endif
