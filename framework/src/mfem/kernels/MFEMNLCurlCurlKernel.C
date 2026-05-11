//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNLCurlCurlKernel.h"
#include "MFEMProblem.h"
#include "NLCurlCurlIntegrator.h"

registerMooseObject("MooseApp", MFEMNLCurlCurlKernel);

InputParameters
MFEMNLCurlCurlKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the nonlinear form "
                             "$(k(\\vec \\nabla x \\vec u) \\vec \\nabla x \\vec u, \\vec \\nabla x \\vec v)_\\Omega "
                             "arising from the weak form of the non-linear operator "
                             "$- \\vec \\nabla x (k(\\vec \\nabla x u) \\vec\\nabla x u)$.");
  params.addParam<MFEMScalarCoefficientName>(
      "k_coefficient", "1.", "Name of property for nonlinear diffusivity coefficient k(\\nabla x u).");
  params.addParam<MFEMVectorCoefficientName>(
      "dk_dcu_coefficient",
      "0.",
      "Name of property partial derivative of diffusivity coefficient k(\\nabla x u) with respect to the "
      "curl of the trial variable u.");
  return params;
}

MFEMNLCurlCurlKernel::MFEMNLCurlCurlKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _k_coef(getScalarCoefficient("k_coefficient")),
    _dk_dcu_coef(getVectorCoefficient("dk_dcu_coefficient")),
    _trial_var(*getMFEMProblem().getGridFunction(getTrialVariableName()))
{
}

mfem::NonlinearFormIntegrator *
MFEMNLCurlCurlKernel::createNLIntegrator()
{
  return new Moose::MFEM::NLCurlCurlIntegrator(_k_coef, _dk_dcu_coef, &_trial_var);
}

#endif
