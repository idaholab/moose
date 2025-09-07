//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSpecificEnthalpyFromPressureTemperatureDirichletBC.h"
#include "SinglePhaseFluidProperties.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", FVSpecificEnthalpyFromPressureTemperatureDirichletBC);

InputParameters
FVSpecificEnthalpyFromPressureTemperatureDirichletBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object.");
  params.addRequiredParam<MooseFunctorName>(
      NS::pressure, "The pressure (units should be based on fluid properties object)");
  params.addRequiredParam<MooseFunctorName>(
      NS::T_fluid, "The temperature (units should be based on fluid properties object)");
  params.addClassDescription(
      "Computes the specific enthalpy at the boundary from the pressure and temperature.");
  return params;
}

FVSpecificEnthalpyFromPressureTemperatureDirichletBC::
    FVSpecificEnthalpyFromPressureTemperatureDirichletBC(const InputParameters & parameters)
  : FVDirichletBCBase(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _pressure(getFunctor<ADReal>(NS::pressure)),
    _temperature(getFunctor<ADReal>(NS::T_fluid))
{
}

ADReal
FVSpecificEnthalpyFromPressureTemperatureDirichletBC::boundaryValue(
    const FaceInfo & fi, const Moose::StateArg & state) const
{
  auto sfa = singleSidedFaceArg(&fi);
  return _fp.h_from_p_T(_pressure(sfa, state), _temperature(sfa, state));
}
