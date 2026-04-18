//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMixedScalarCurlKernel.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", MixedScalarCurlKernel);

namespace Moose::MFEM
{
InputParameters
MixedScalarCurlKernel::validParams()
{
  InputParameters params = MixedBilinearFormKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k\\vec\\nabla \\times \\vec u, v)_\\Omega$ "
                             "arising from the weak form of the scalar curl operator "
                             "$k\\vec\\nabla \\times u$. The vector must be 2D.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar property k to multiply the integrator by.");
  return params;
}

MixedScalarCurlKernel::MixedScalarCurlKernel(const InputParameters & parameters)
  : MixedBilinearFormKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
}

mfem::BilinearFormIntegrator *
MixedScalarCurlKernel::createMBFIntegrator()
{
  return new mfem::MixedScalarCurlIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
