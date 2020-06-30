//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADEnergySUPG.h"

registerMooseObject("NavierStokesApp", INSADEnergySUPG);

InputParameters
INSADEnergySUPG::validParams()
{
  InputParameters params = ADKernelSUPG::validParams();
  params.addClassDescription("Adds the supg stabilization to the INS temperature/energy equation");
  params.set<MaterialPropertyName>("tau_name") = "tau_energy";
  return params;
}

INSADEnergySUPG::INSADEnergySUPG(const InputParameters & parameters)
  : ADKernelSUPG(parameters),
    _temperature_strong_residual(getADMaterialProperty<Real>("temperature_strong_residual"))
{
}

ADReal
INSADEnergySUPG::precomputeQpStrongResidual()
{
  return _temperature_strong_residual[_qp];
}
