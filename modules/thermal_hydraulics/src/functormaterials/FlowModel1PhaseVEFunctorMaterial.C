//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModel1PhaseVEFunctorMaterial.h"

registerMooseObject("ThermalHydraulicsApp", FlowModel1PhaseVEFunctorMaterial);

InputParameters
FlowModel1PhaseVEFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();

  params.addClassDescription("Computes several quantities for FlowModel1Phase.");

  return params;
}

FlowModel1PhaseVEFunctorMaterial::FlowModel1PhaseVEFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rhoA(getFunctor<ADReal>(THM::RHOA)),
    _rhouA(getFunctor<ADReal>(THM::RHOUA)),
    _rhoEA(getFunctor<ADReal>(THM::RHOEA)),
    _A(getFunctor<Real>(THM::AREA))
{
  // specific volume
  addSpecificVolumeFunctorProperty<true>();
  addSpecificVolumeFunctorProperty<false>();

  // specific internal energy
  addSpecificVolumeFunctorProperty<true>();
  addSpecificVolumeFunctorProperty<false>();
}
