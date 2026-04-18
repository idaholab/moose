//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNLDiffusionKernel.h"
#include "MFEMProblem.h"
#include "NLDiffusionIntegrator.h"

registerMooseMFEMObject("MooseApp", NLDiffusionKernel);

namespace Moose::MFEM
{
InputParameters
NLDiffusionKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the nonlinear form "
                             "$(k(u) \\vec\\nabla u, \\vec\\nabla v)_\\Omega "
                             "arising from the weak form of the non-linear operator "
                             "$- \\vec\\nabla \\cdot (k(u) \\vec\\nabla u)$.");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "k_coefficient", "1.", "Name of property for nonlinear diffusivity coefficient k(u).");
  params.addParam<Moose::MFEM::ScalarCoefficientName>(
      "dk_du_coefficient",
      "0.",
      "Name of property partial derivative of diffusivity coefficient k(u) with respect to the "
      "trial variable u.");
  return params;
}

NLDiffusionKernel::NLDiffusionKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _k_coef(getScalarCoefficient("k_coefficient")),
    _dk_du_coef(getScalarCoefficient("dk_du_coefficient")),
    _trial_var(*getMFEMProblem().getGridFunction(getTrialVariableName()))
{
}

mfem::NonlinearFormIntegrator *
NLDiffusionKernel::createNLIntegrator()
{
  return new NLDiffusionIntegrator(_k_coef, _dk_du_coef, &_trial_var);
}

} // namespace Moose::MFEM
#endif
