//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMVectorFEMassKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorFEMassKernel);

InputParameters
MFEMVectorFEMassKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k \\vec u, \\vec v)_\\Omega$ "
                             "arising from the weak form of the mass operator "
                             "$k \\vec u$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property k to multiply the Laplacian by");
  return params;
}

MFEMVectorFEMassKernel::MFEMVectorFEMassKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    // FIXME: The MFEM bilinear form can also handle vector and matrix
    // coefficients, so ideally we'd handle all three too.
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

mfem::BilinearFormIntegrator *
MFEMVectorFEMassKernel::createBFIntegrator()
{
  return new mfem::VectorFEMassIntegrator(_coef);
}

#endif
