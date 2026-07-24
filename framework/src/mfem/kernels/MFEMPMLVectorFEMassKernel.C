//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPMLVectorFEMassKernel.h"

registerMooseObject("MooseApp", MFEMPMLVectorFEMassKernel);

InputParameters
MFEMPMLVectorFEMassKernel::validParams()
{
  InputParameters params = MFEMPMLKernel::validParams();
  params.addClassDescription(
      "Adds a perfectly-matched-layer-stretched vector FE mass integrator, applying the complex "
      "coordinate stretch to the base coefficient of the bilinear form $(k\\vec u, \\vec v)$.");
  return params;
}

MFEMPMLVectorFEMassKernel::MFEMPMLVectorFEMassKernel(const InputParameters & parameters)
  : MFEMPMLKernel(parameters, MFEMPMLDiagMatrixCoefficient::C2)
{
}

#endif
