//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorRadiativeBC.h"

registerMooseObject("HeatConductionApp", FVFunctorRadiativeBC);

InputParameters
FVFunctorRadiativeBC::validParams()
{
  InputParameters params = FVRadiativeHeatFluxBCBase::validParams();
  params.addClassDescription("Boundary condition for radiative heat exchange where the emissivity "
                             "function is supplied by a Function.");
  params.addRequiredParam<MooseFunctorName>(
      "emissivity", "Functor describing emissivity for radiative boundary condition");
  return params;
}

FVFunctorRadiativeBC::FVFunctorRadiativeBC(const InputParameters & parameters)
  : FVRadiativeHeatFluxBCBase(parameters), _emissivity(getFunctor<Real>("emissivity"))
{
}

Real
FVFunctorRadiativeBC::coefficient() const
{
  return _emissivity(singleSidedFaceArg(), determineState());
}
