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
  params.addParam<bool>(
      "multiply_by_mass",
      false,
      "Whether the rate should be multiplied by the kinetic_species mass (in grams)");
  params.addParam<Real>("kinetic_molal_index",
                        0.0,
                        "The rate is multiplied by kinetic_species_molality^kinetic_molal_index / "
                        "(kinetic_species_molality^kinetic_molal_index + "
                        "kinetic_half_saturation^kinetic_molal_index)^kinetic_monod_index");
  params.addParam<Real>("kinetic_monod_index",
                        0.0,
                        "The rate is multiplied by kinetic_species_molality^kinetic_molal_index / "
                        "(kinetic_species_molality^kinetic_molal_index + "
                        "kinetic_half_saturation^kinetic_molal_index)^kinetic_monod_index");
  params.addParam<Real>("kinetic_half_saturation",
                        0.0,
                        "The rate is multiplied by kinetic_species_molality^kinetic_molal_index / "
                        "(kinetic_species_molality^kinetic_molal_index + "
                        "kinetic_half_saturation^kinetic_molal_index)^kinetic_monod_index");
  params.addParam<std::vector<std::string>>("promoting_species_names",
                                            "Names of any promoting species");
  params.addParam<std::vector<Real>>("promoting_indices", "Indices of the promoting species");
  params.addParam<std::vector<Real>>(
      "promoting_monod_indices",
      "Indices of the monod denominators of the promoting species.  If not given, then the default "
      "is 0 for each promoting species, meaning that there is no monod form");
  params.addParam<std::vector<Real>>("promoting_half_saturation",
                                     "Half-saturation constants for the monod expression.  If not "
                                     "given, then the default is 0 for each promoting species");
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
  MooseEnum direction("both dissolution precipitation raw death", "both");
  params.addParam<MooseEnum>(
      "direction",
      direction,
      "Direction of reaction.  Let Q = the activity product of the kinetic reaction, and K = the "
      "equilibrium constant of the reaction.  Then direction means the following.  both = "
      "dissolution and precipitation are allowed.  (Specifically, if Q < K then dissolution will "
      "occur, that is, the kinetic species mass will decrease with time.  If Q > K then "
      "precipitation will occur, that is, the kinetic species mass will increase with time.)  "
      "dissolution = if Q < K then dissolution will occur, and when Q > K then the rate will be "
      "set to zero so that precipitation will be prevented.  precipitation = if Q > K then "
      "precipitation will occur, and when Q < K then the rate will be set to zero so that "
      "dissolution will be prevented.  raw = the rate will not depend on sgn(1 - (Q/K)), which "
      "means dissolution will occur if intrinsic_rate_constant > 0, and precipitation will occur "
      "when intrinsic_rate_constant < 0.  death = the rate will not depend on sgn(1 - (Q/K)), "
      "which means dissolution will occur if intrinsic_rate_constant > 0, and precipitation will "
      "occur when intrinsic_rate_constant < 0, and, in addition, no reactants will be produced or "
      "consumed by this kinetic reaction (only the kinetic species mass will change).");
  params.addParam<std::string>(
      "non_kinetic_biological_catalyst",
      "H2O",
      "Name of the primary or equilibrium species that acts as a biological catalyst.");
  params.addParam<Real>(
      "non_kinetic_biological_efficiency",
      0.0,
      "When one mole of the kinetic species dissolves, non_kinetic_biological_efficiency moles of "
      "the non_kinetic_biological_catalyst is created");
  params.addParam<Real>(
      "kinetic_biological_efficiency",
      -1,
      "This is used when modelling biologically-catalysed reactions, when the biomass is treated "
      "as a kinetic species, and the reactants and reactant-products are in equilibrium in the "
      "aqueous solution.  When one mole of reaction is catalysed, the biomass increases by "
      "kinetic_biological_efficiency moles");
  params.addParam<Real>(
      "energy_captured",
      0.0,
      "In biologically-catalysed kinetic reactions, this is the energy captured by the cell, per "
      "mol of reaction turnover.  Specifically, for each mole of kinetic reaction, the microbe "
      "will produce m moles of ATP via a reaction such as ADP + PO4--- -> ATP + H2O, with "
      "free-energy change G (usually around 45 kJ/mol).  Then, energy_captured = m * G.  For "
      "non-biologically-catalysed reactions, this should be zero.  The impact of energy_captured "
      "is that the reaction's equilibrium constant is K_database * exp(-energy_captured / R / "
      "T_in_Kelvin)");
  params.addClassDescription(
      "User object that defines a kinetic rate.  Note that more than one rate can be prescribed to "
      "a single kinetic_species: the sum the individual rates defines the overall rate.  "
      "GeochemistryKineticRate simply specifies the algebraic form for a kinetic rate: to actually "
      "use it in a calculation, you must use it in the GeochemicalModelDefinition.  The rate is "
      "intrinsic_rate_constant * area_quantity * (optionally, mass of kinetic_species in "
      "grams) * kinetic_molality^kinetic_molal_index / (kinetic_molality^kinetic_molal_index + "
      "kinetic_half_saturation^kinetic_molal_index)^kinetic_monod_index * "
      "(product_over_promoting_species m^promoting_index / (m^promoting_index + "
      "promoting_half_saturation^promiting_index)^promoting_monod_index) * |1 - (Q/K)^theta|^eta * "
      "exp(activation_energy / R * (1/T0 - 1/T)) * Direction(1 - (Q/K)).  Please see the markdown "
      "documentation for examples");

  return params;
}

GeochemistryKineticRate::GeochemistryKineticRate(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _promoting_names(getParam<std::vector<std::string>>("promoting_species_names")),
    _monod_ind(isParamValid("promoting_monod_indices")
                   ? getParam<std::vector<Real>>("promoting_monod_indices")
                   : std::vector<Real>(_promoting_names.size(), 0.0)),
    _half_sat(isParamValid("promoting_half_saturation")
                  ? getParam<std::vector<Real>>("promoting_half_saturation")
                  : std::vector<Real>(_promoting_names.size(), 0.0)),
    _rate_description(getParam<std::string>("kinetic_species_name"),
                      getParam<Real>("intrinsic_rate_constant"),
                      getParam<Real>("area_quantity"),
                      getParam<bool>("multiply_by_mass"),
                      getParam<Real>("kinetic_molal_index"),
                      getParam<Real>("kinetic_monod_index"),
                      getParam<Real>("kinetic_half_saturation"),
                      _promoting_names,
                      getParam<std::vector<Real>>("promoting_indices"),
                      _monod_ind,
                      _half_sat,
                      getParam<Real>("theta"),
                      getParam<Real>("eta"),
                      getParam<Real>("activation_energy"),
                      getParam<Real>("one_over_T0"),
                      getParam<MooseEnum>("direction").getEnum<DirectionChoiceEnum>(),
                      getParam<std::string>("non_kinetic_biological_catalyst"),
                      getParam<Real>("non_kinetic_biological_efficiency"),
                      getParam<Real>("kinetic_biological_efficiency"),
                      getParam<Real>("energy_captured"))
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
