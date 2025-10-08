//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTimeDerivativeDiffusionKernel.h"

registerMooseObject("MooseApp", MFEMTimeDerivativeDiffusionKernel);

InputParameters
MFEMTimeDerivativeDiffusionKernel::validParams()
{
  InputParameters params = MFEMDiffusionKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k\\vec\\nabla \\dot{u}, \\vec\\nabla v)_\\Omega$ "
                             "arising from the weak form of the Laplacian operator "
                             "$- \\vec\\nabla \\cdot \\left( k \\vec \\nabla \\dot{u} \\right)$.");  
  return params;
}

MFEMTimeDerivativeDiffusionKernel::MFEMTimeDerivativeDiffusionKernel(
    const InputParameters & parameters)
  : MFEMDiffusionKernel(parameters),
    _var_dot_name(Moose::MFEM::CreateTimeDerivativeName(_test_var_name))
{
}

#endif
