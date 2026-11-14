//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVConservativeDiffuseInterfaceEnthalpyBC.h"
#include "ConservativeDiffuseInterfaceMixtureEnthalpy.h"
#include "NSFVUtils.h"

registerMooseObject("NavierStokesApp", LinearFVConservativeDiffuseInterfaceEnthalpyBC);

InputParameters
LinearFVConservativeDiffuseInterfaceEnthalpyBC::validParams()
{
  InputParameters params = LinearFVBoundaryCondition::validParams();
  params.addClassDescription("Temperature or heat-flux boundary condition on the conserved mixture "
                             "enthalpy density of a diffuse-interface flow.");
  params.addRequiredParam<UserObjectName>(
      "mixture_enthalpy", "The ConservativeDiffuseInterfaceMixtureEnthalpy relating rho*h and T.");
  params.addRequiredParam<MooseFunctorName>("volume_fraction", "The liquid volume fraction.");
  params.addParam<MooseFunctorName>("temperature", "The boundary temperature.");
  params.addParam<MooseFunctorName>(
      "heat_flux", "0", "The heat flux into the domain, used when no temperature is given.");
  return params;
}

LinearFVConservativeDiffuseInterfaceEnthalpyBC::LinearFVConservativeDiffuseInterfaceEnthalpyBC(
    const InputParameters & parameters)
  : LinearFVBoundaryCondition(parameters),
    _mixture(getUserObject<ConservativeDiffuseInterfaceMixtureEnthalpy>("mixture_enthalpy")),
    _alpha(getFunctor<Real>("volume_fraction")),
    _temperature(isParamValid("temperature") ? &getFunctor<Real>("temperature") : nullptr),
    _heat_flux(getFunctor<Real>("heat_flux"))
{
  if (_temperature && isParamSetByUser("heat_flux"))
    paramError("heat_flux", "A boundary takes either a temperature or a heat flux, not both.");
}

Real
LinearFVConservativeDiffuseInterfaceEnthalpyBC::boundaryTemperature() const
{
  mooseAssert(_temperature, "No boundary temperature is prescribed");
  return (*_temperature)(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVConservativeDiffuseInterfaceEnthalpyBC::heatFlux() const
{
  return _heat_flux(singleSidedFaceArg(_current_face_info), determineState());
}

Real
LinearFVConservativeDiffuseInterfaceEnthalpyBC::computeBoundaryValue() const
{
  const auto & elem_info = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);
  if (!_temperature)
    return _var.getElemValue(elem_info, determineState());

  const auto face = singleSidedFaceArg(_current_face_info);
  return _mixture.enthalpyDensity(_alpha(face, determineState()), boundaryTemperature(), face);
}

Real
LinearFVConservativeDiffuseInterfaceEnthalpyBC::computeBoundaryNormalGradient() const
{
  const auto & elem_info = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);
  return (computeBoundaryValue() - _var.getElemValue(elem_info, determineState())) /
         computeCellToFaceDistance();
}
