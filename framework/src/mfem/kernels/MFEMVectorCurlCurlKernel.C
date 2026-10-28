//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorCurlCurlKernel.h"

registerMooseObject("MooseApp", MFEMVectorCurlCurlKernel);

InputParameters
MFEMVectorCurlCurlKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form $(k \\vec\\nabla "
      "\\times \\vec u, \\vec\\nabla \\times \\vec v)_\\Omega$ arising from the weak form of the "
      "operator $\\vec\\nabla \\times \\left(k \\vec\\nabla \\times \\vec u\\right)$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMVectorCurlCurlKernel::MFEMVectorCurlCurlKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
MFEMVectorCurlCurlKernel::createBFIntegrator()
{
  return new mfem::VectorCurlCurlIntegrator(_coef);
}

#endif
