//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMatNonlinearDiffusion.h"

registerKokkosResidualObject("KokkosTrainingApp", KokkosMatNonlinearDiffusion);

InputParameters
KokkosMatNonlinearDiffusion::validParams()
{
  InputParameters params = KernelGrad::validParams();
  params.addClassDescription("Nonlinear diffusion with the conductivity and its optional "
                             "derivative provided by material properties.");
  params.addParam<MaterialPropertyName>(
      "conductivity", "conductivity", "Property name of the conductivity");
  params.addParam<MaterialPropertyName>(
      "conductivity_derivative",
      "conductivity_derivative",
      "Property name of the derivative of the conductivity with respect to the variable");
  return params;
}

KokkosMatNonlinearDiffusion::KokkosMatNonlinearDiffusion(const InputParameters & parameters)
  : KernelGrad(parameters)
{
  _k = getKokkosMaterialProperty<Real>("conductivity");

  if (hasKokkosMaterialProperty<Real>("conductivity_derivative"))
    _dk_du = getKokkosMaterialProperty<Real>("conductivity_derivative");
}
