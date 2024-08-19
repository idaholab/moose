//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoPhaseNCGPartialPressureFluidProperties.h"
#include "IdealRealGasMixtureFluidProperties.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", TwoPhaseNCGPartialPressureFluidProperties);

InputParameters
TwoPhaseNCGPartialPressureFluidProperties::validParams()
{
  InputParameters params = TwoPhaseNCGFluidProperties::validParams();

  params.addClassDescription(
      "Two-phase fluid with single NCG using partial pressure mixture model");

  params.addRequiredParam<UserObjectName>("fp_ncg", "NCG fluid properties object");

  return params;
}

TwoPhaseNCGPartialPressureFluidProperties::TwoPhaseNCGPartialPressureFluidProperties(
    const InputParameters & parameters)
  : TwoPhaseNCGFluidProperties(parameters),

    _fp_ncg(getUserObject<SinglePhaseFluidProperties>("fp_ncg"))
{
  _fp_2phase = &_fe_problem.getUserObject<TwoPhaseFluidProperties>(_2phase_name);
  _fp_vapor_primary = &_fe_problem.getUserObject<SinglePhaseFluidProperties>(getVaporName());

  // create vapor mixture fluid properties
  if (_tid == 0)
  {
    const std::string class_name = "IdealRealGasMixtureFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    params.set<UserObjectName>("fp_primary") = getVaporName();
    params.set<std::vector<UserObjectName>>("fp_secondary") = {getParam<UserObjectName>("fp_ncg")};
    _fe_problem.addUserObject(class_name, _vapor_mixture_name, params);
  }
  _fp_vapor_mixture = &_fe_problem.getUserObject<VaporMixtureFluidProperties>(_vapor_mixture_name);

  _molar_masses = {_fp_vapor_primary->molarMass(), _fp_ncg.molarMass()};
}

Real
TwoPhaseNCGPartialPressureFluidProperties::x_sat_ncg_from_p_T(Real p, Real T) const
{
  const auto molar_fraction_ncg = (p - _fp_2phase->p_sat(T)) / p;
  const std::vector<Real> molar_fractions = {1.0 - molar_fraction_ncg, molar_fraction_ncg};
  const auto mass_fractions =
      _fp_vapor_mixture->massFractionsFromMolarFractions(molar_fractions, _molar_masses);
  return mass_fractions[1];
}
