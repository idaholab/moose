//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPMLCurlCurlKernel.h"

registerMooseObject("MooseApp", MFEMPMLCurlCurlKernel);

InputParameters
MFEMPMLCurlCurlKernel::validParams()
{
  InputParameters params = MFEMPMLKernel::validParams();
  params.addClassDescription(
      "Adds a perfectly-matched-layer-stretched curl-curl integrator, applying the complex "
      "coordinate stretch to the base coefficient of the bilinear form "
      "$(k\\vec\\nabla\\times\\vec u, \\vec\\nabla\\times\\vec v)$.");
  return params;
}

MFEMPMLCurlCurlKernel::MFEMPMLCurlCurlKernel(const InputParameters & parameters)
  : MFEMPMLKernel(parameters, MFEMPMLMatrixCoefficient::CURL)
{
}

#endif
