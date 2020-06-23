//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryKineticRate.h"

registerMooseObject("GeochemistryApp", GeochemistryKineticRate);

InputParameters
GeochemistryKineticRate::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::string>(
      "kinetic_species_name",
      "The name of the kinetic species that will be controlled by this rate");
  params.addRequiredRangeCheckedParam<Real>("intrinsic_rate_constant",
                                            "intrinsic_rate_constant >= 0.0",
                                            "The intrinsic rate constant for the reaction");
  params.addRangeCheckedParam<Real>(
      "area_quantity",
      1.0,
      "area_quantity > 0.0",
      "The surface area of the kinetic species in m^2 (if multiply_by_mass = false) or the "
      "specific surface area of the kinetic species in m^2/g (if multiply_by_mass = true)");
  params.addParam<bool>("multiply_by_mass",
                        false,
                        "Whether the rate should be multiplied by the kinetic species mass");
  params.addParam<std::vector<std::string>>("promoting_species_names",
                                            "Names of any promoting species");
  params.addParam<std::vector<Real>>("promoting_species_indices",
                                     "Indices of the promoting species");
  params.addParam<Real>("theta", 1.0, "Theta parameter, which appears in |1 - (Q/K)^theta|^eta");
  params.addParam<Real>("eta", 1.0, "Eta parameter, which appears in |1 - (Q/K)^theta|^eta");
  params.addRangeCheckedParam<Real>(
      "activation_energy",
      0.0,
      "activation_energy >= 0.0",
      "Activation energy, in J.mol^-1, which appears in exp(activation_energy / R * (1/T0 - 1/T))");
  params.addParam<Real>(
      "one_over_T0",
      0.0,
      "1/T0, in 1/Kelvin, which appears in exp(activation_energy / R * (1/T0 - 1/T))");
  params.addClassDescription(
      "User object that defines a kinetic rate.  Note that more than one rate can be prescribed to "
      "a single kinetic_species: the sum the individual rates defines the overall rate.  "
      "GeochemistryKineticRate simply specifies the algebraic form for a kinetic rate: to actually "
      "use it in a calculation, you must use it in the GeochemicalModelDefinition.  The rate is "
      "intrinsic_rate_constant * area_quantity * (mass of kinetic_species in grams, optionally) * "
      "(product_over_promoting_species m^promoting_species_index) * |1 - (Q/K)^theta|^eta * "
      "exp(activation_energy / R * (1/T0 - 1/T)) * sign(1 - (Q/K)).  Please see the markdown "
      "documentation for examples");

  return params;
}

GeochemistryKineticRate::GeochemistryKineticRate(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _rate_description(getParam<std::string>("kinetic_species_name"),
                      getParam<Real>("intrinsic_rate_constant"),
                      getParam<Real>("area_quantity"),
                      getParam<bool>("multiply_by_mass"),
                      getParam<std::vector<std::string>>("promoting_species_names"),
                      getParam<std::vector<Real>>("promoting_species_indices"),
                      getParam<Real>("theta"),
                      getParam<Real>("eta"),
                      getParam<Real>("activation_energy"),
                      getParam<Real>("one_over_T0"))
{
}

void
GeochemistryKineticRate::initialize()
{
}

void
GeochemistryKineticRate::execute()
{
}

void
GeochemistryKineticRate::finalize()
{
}

const KineticRateUserDescription &
GeochemistryKineticRate::getRateDescription() const
{
  return _rate_description;
}
