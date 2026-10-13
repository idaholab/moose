//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeOuterWallRadiationScalarKernel.h"

#include "ADFunctorInterface.h"
#include "ThermalSolidProperties.h"
#include "PhysicalConstants.h"

registerMooseObject("ThermalHydraulicsApp", ADPipeOuterWallRadiationScalarKernel);

InputParameters
ADPipeOuterWallRadiationScalarKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params += ADFunctorInterface::validParams();
  params.addClassDescription(
      "Adds a radiative heat transfer term, exchanged with an ambient/surrounding "
      "environment, to the outer-surface radial node of a pipe wall.");
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  params.addRequiredParam<MooseFunctorName>("area",
                                            "Cross-sectional area of this wall layer [m^2]");
  params.addRequiredParam<MooseFunctorName>(
      "perimeter", "Perimeter of this wall node exposed to the radiative environment [m]");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor [K]");
  params.addRequiredParam<MooseFunctorName>("emissivity", "Emissivity functor");
  params.addParam<MooseFunctorName>("view_factor", 1.0, "View factor functor");
  params.addParam<MooseFunctorName>(
      "scale", 1.0, "Functor by which to scale the radiative heat transfer term");
  params.addParam<Real>("stefan_boltzmann_constant",
                        PhysicalConstants::stefan_boltzmann_constant,
                        "Stefan-Boltzmann constant");

  return params;
}

ADPipeOuterWallRadiationScalarKernel::ADPipeOuterWallRadiationScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    ADFunctorInterface(this),
    _sp(this->template getUserObject<ThermalSolidProperties>("sp")),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _area(this->template getFunctor<ADReal>("area")),
    _perimeter(this->template getFunctor<ADReal>("perimeter")),
    _T_ambient(this->template getFunctor<ADReal>("T_ambient")),
    _emissivity(this->template getFunctor<ADReal>("emissivity")),
    _view_factor(this->template getFunctor<ADReal>("view_factor")),
    _scale(this->template getFunctor<ADReal>("scale")),
    _sigma(this->template getParam<Real>("stefan_boltzmann_constant"))
{
}

ADReal
ADPipeOuterWallRadiationScalarKernel::computeQpResidual()
{
  const Moose::ElemArg elem_arg = Moose::ElemArg();
  const int i = 0;
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();

  const auto A = _area(elem_arg, state);
  const auto cp = _sp.cp_from_T(Base::_u[i]);
  const auto rho = _sp.rho_from_T(Base::_u[i]);

  const auto scale = _scale(elem_arg, state);
  const auto emissivity = _emissivity(elem_arg, state);
  const auto view_factor = _view_factor(elem_arg, state);
  const auto T4 = pow(Base::_u[i], 4);
  const auto T4inf = pow(_T_ambient(elem_arg, state), 4);

  // Radiative heat transfer with the ambient/surrounding environment
  const auto q_radiation =
      _sigma * scale * emissivity * view_factor * _perimeter(elem_arg, state) * (T4inf - T4);

  return -q_radiation / A / rho / cp;
}

Real
ADPipeOuterWallRadiationScalarKernel::computeQpJacobian()
{
  mooseError("computeQpJacobian() should not be called in AD mode");
  return 0;
}
