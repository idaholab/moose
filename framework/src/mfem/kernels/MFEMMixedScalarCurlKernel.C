//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMMixedScalarCurlKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMixedScalarCurlKernel);

InputParameters
MFEMMixedScalarCurlKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k\\vec\\nabla \\times \\vec u, v)_\\Omega$ "
                             "arising from the weak form of the scalar curl operator "
                             "$k\\vec\\nabla \\times u$. The vector must be 2D.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar property k to multiply the integrator by.");
  return params;
}

MFEMMixedScalarCurlKernel::MFEMMixedScalarCurlKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedScalarCurlKernel::createMBFIntegrator()
{
  return new mfem::MixedScalarCurlIntegrator(_coef);
}

#endif
