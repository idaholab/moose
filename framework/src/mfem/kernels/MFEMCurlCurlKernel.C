//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCurlCurlKernel.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", CurlCurlKernel);

namespace Moose::MFEM
{
InputParameters
CurlCurlKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Adds the domain integrator to an MFEM problem for the bilinear form "
      "$(k\\vec\\nabla \\times \\vec u, \\vec\\nabla \\times \\vec v)_\\Omega$ "
      "arising from the weak form of the curl curl operator "
      "$k\\vec\\nabla \\times \\vec\\nabla \\times \\vec u$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "coefficient", "1.", "Name of scalar coefficient k to multiply the integrator by.");
  return params;
}

CurlCurlKernel::CurlCurlKernel(const InputParameters & parameters)
  : Kernel(parameters), _coef(getScalarCoefficient("coefficient"))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
}

mfem::BilinearFormIntegrator *
CurlCurlKernel::createBFIntegrator()
{
  return new mfem::CurlCurlIntegrator(_coef);
}

} // namespace Moose::MFEM
#endif
