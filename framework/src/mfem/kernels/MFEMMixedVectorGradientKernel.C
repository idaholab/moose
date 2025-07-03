//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedVectorGradientKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMixedVectorGradientKernel);

InputParameters
MFEMMixedVectorGradientKernel::validParams()
{
  InputParameters params = MFEMMixedBilinearFormKernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the mixed bilinear form "
      "$(k\\vec\\nabla u, \\vec v)_\\Omega$ "
      "arising from the weak form of the gradient operator "
      "$k\\vec \\nabla u$.");
  params.addParam<MFEMScalarCoefficientName>("coefficient", "1.", "Name of property k to use.");
  return params;
}

MFEMMixedVectorGradientKernel::MFEMMixedVectorGradientKernel(const InputParameters & parameters)
  : MFEMMixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
}

std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
MFEMMixedVectorGradientKernel::createBFIntegrator()
{
  if (getParam<MooseEnum>("numeric_type") == "complex")
    mooseError("Complex-valued mixed kernels are not supported.");

  return std::make_pair(new mfem::MixedVectorGradientIntegrator(_coef), nullptr);
}

#endif
