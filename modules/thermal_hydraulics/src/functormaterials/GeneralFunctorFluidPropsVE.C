//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralFunctorFluidPropsVE.h"
#include "NS.h"

registerMooseObject("ThermalHydraulicsApp", GeneralFunctorFluidPropsVE);

InputParameters
GeneralFunctorFluidPropsVE::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Functor material to access the fluid properties and ");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");

  // THM defines v & e already as aux-variables
  params.addParam<MooseFunctorName>("e", "Specific internal energy");
  params.addParam<MooseFunctorName>("v", "Specific volume");

  // THM uses these three variables as main variables
  params.addParam<MooseFunctorName>("rhoA", "Functor for rho * A");
  params.addParam<MooseFunctorName>("rhoEA", "Functor for rho * E * A");
  params.addParam<MooseFunctorName>("rhouA", "Functor for rho * u * A");

  // These are used to compute v,e from the conserved variables
  params.addParam<MooseFunctorName>("alpha", "Functor describing the local phase fraction");
  params.addParam<MooseFunctorName>("A", "Functor describing the local area");

  // Avoid overlapping with existing functor names
  params.addParam<MooseFunctorName>(
      "pressure_name", NS::pressure, "Name for the pressure functor declared");
  params.addParam<MooseFunctorName>(
      "temperature_name", NS::temperature, "Name for the temperature functor declared");

  return params;
}

GeneralFunctorFluidPropsVE::GeneralFunctorFluidPropsVE(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _e(isParamValid("e") ? &getFunctor<ADReal>("e") : nullptr),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _rhoA(isParamValid("rhoA") ? &getFunctor<ADReal>("rhoA") : nullptr),
    _rhoEA(isParamValid("rhoEA") ? &getFunctor<ADReal>("rhoEA") : nullptr),
    _rhouA(isParamValid("rhouA") ? &getFunctor<ADReal>("rhouA") : nullptr),
    _A(isParamValid("A") ? &getFunctor<ADReal>("A") : nullptr),
    _alpha(isParamValid("alpha") ? &getFunctor<ADReal>("alpha") : nullptr)
{
  // Only allow one set of input variables
  bool ve_provided = isParamValid("e") && isParamValid("v");
  bool conserved_provided = isParamValid("rhoA") && isParamValid("rhoEA") &&
                            isParamValid("rhouA") && isParamValid("A") && isParamValid("alpha");

  if (!ve_provided && !conserved_provided)
    mooseError("One of (v,e) and (rhoA, rhoEA, rhouA, A, alpha) parameter groups must be provided");
  if (ve_provided && conserved_provided)
    mooseError(
        "Only one of (v,e) and (rhoA, rhoEA, rhouA, A, alpha) parameter groups can be provided");

  if (conserved_provided)
  {
    addFunctorProperty<ADReal>(name() + "_helper_e",
                               [this](const auto & r, const auto & t) -> ADReal
                               {
                                 return ((*_rhoEA)(r, t) -
                                         0.5 * (*_rhouA)(r, t) * (*_rhouA)(r, t) / (*_rhoA)(r, t)) /
                                        (*_rhoA)(r, t);
                               });
    addFunctorProperty<ADReal>(name() + "_helper_v",
                               [this](const auto & r, const auto & t) -> ADReal
                               { return (*_alpha)(r, t) * (*_A)(r, t) / (*_rhoA)(r, t); });
    _e = &getFunctor<ADReal>(name() + "_helper_e");
    _v = &getFunctor<ADReal>(name() + "_helper_v");
  }

  addFunctorProperty<ADReal>(getParam<MooseFunctorName>("pressure_name"),
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.p_from_v_e((*_v)(r, t), (*_e)(r, t)); });
  addFunctorProperty<ADReal>(getParam<MooseFunctorName>("temperature_name"),
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.T_from_v_e((*_v)(r, t), (*_e)(r, t)); });
  addFunctorProperty<ADReal>("c",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.c_from_v_e((*_v)(r, t), (*_e)(r, t)); });
  addFunctorProperty<ADReal>("cp",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.cp_from_v_e((*_v)(r, t), (*_e)(r, t)); });
  addFunctorProperty<ADReal>("cv",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.cv_from_v_e((*_v)(r, t), (*_e)(r, t)); });
  addFunctorProperty<ADReal>("mu",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.mu_from_v_e((*_v)(r, t), (*_e)(r, t)); });
  addFunctorProperty<ADReal>("k",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.k_from_v_e((*_v)(r, t), (*_e)(r, t)); });
  addFunctorProperty<ADReal>("s",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.s_from_v_e((*_v)(r, t), (*_e)(r, t)); });
  addFunctorProperty<ADReal>("g",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _fp.g_from_v_e((*_v)(r, t), (*_e)(r, t)); });
}
