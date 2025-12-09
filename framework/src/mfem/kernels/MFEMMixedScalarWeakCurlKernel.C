//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedScalarWeakCurlKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMixedScalarWeakCurlKernel);

InputParameters
MFEMMixedScalarWeakCurlKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k u, \\vec\\nabla \\times \\vec v)_\\Omega$ "
                             "arising from the weak form of the scalar curl operator "
                             "$\\vec\\nabla \\times (k u \\hat z)$. The space must be 2D.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar property k to multiply the integrator by.");
  return params;
}

MFEMMixedScalarWeakCurlKernel::MFEMMixedScalarWeakCurlKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMMixedScalarWeakCurlKernel::createMBFIntegrator()
{
  return new mfem::MixedScalarWeakCurlIntegrator(_coef);
}

#endif
