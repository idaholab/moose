//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDiffusionKernel.h"
#include "MFEMProblem.h"

#include <iomanip>

namespace
{
const std::string COEFFICIENT = "coefficient";
const std::string MAT_COEFFICIENT = "matrix_coefficient";
}

registerMooseObject("MooseApp", MFEMDiffusionKernel);

InputParameters
MFEMDiffusionKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k\\vec\\nabla u, \\vec\\nabla v)_\\Omega$ "
                             "arising from the weak form of the Laplacian operator "
                             "$- \\vec\\nabla \\cdot \\left( k \\vec \\nabla u \\right)$.");
  params.addParam<MFEMScalarCoefficientName>(
      COEFFICIENT, "1.", "Name of property for diffusion coefficient k.");
  params.addParam<MFEMMatrixCoefficientName>(
      MAT_COEFFICIENT, "1.", "Name of property for matrix diffusion coefficient Q.");
  return params;
}

MFEMDiffusionKernel::MFEMDiffusionKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient(COEFFICIENT)),
    _matrix_coef(getMatrixCoefficient(MAT_COEFFICIENT))
// FIXME: The MFEM bilinear form can also handle vector and matrix
// coefficients, so ideally we'd handle all three too.
{
  if (parameters.isParamSetByUser(COEFFICIENT) && parameters.isParamSetByUser(MAT_COEFFICIENT))
  {
    mooseError("You must specify only one of parameter ",
               std::quoted(COEFFICIENT),
               " and ",
               std::quoted(MAT_COEFFICIENT));
  }
}

mfem::BilinearFormIntegrator *
MFEMDiffusionKernel::createBFIntegrator()
{
  if (_pars.isParamSetByUser(COEFFICIENT))
  {
    return new mfem::DiffusionIntegrator(_coef);
  }
  if (_pars.isParamSetByUser(MAT_COEFFICIENT))
  {
    return new mfem::DiffusionIntegrator(_matrix_coef);
  }
  mooseError("You must specify exactly one of parameter ",
             std::quoted(COEFFICIENT),
             " and ",
             std::quoted(MAT_COEFFICIENT));
}

#endif
