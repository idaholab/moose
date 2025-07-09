//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMMassKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMassKernel);

InputParameters
MFEMMassKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the bilinear form "
                             "$(k u, v)_\\Omega$ "
                             "arising from the weak form of the mass operator "
                             "$ku$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of property for the mass coefficient k.");
  return params;
}

MFEMMassKernel::MFEMMassKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient")))
{
}

mfem::BilinearFormIntegrator *
MFEMMassKernel::createBFIntegrator()
{
  return new mfem::MassIntegrator(_coef);
}

#endif
