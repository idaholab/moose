//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryReactorBase.h"

InputParameters
GeochemistryReactorBase::sharedParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<std::string>>(
      "swap_out_of_basis",
      "Species that should be removed from the model_definition's basis and be replaced with the "
      "swap_into_basis species");
  params.addParam<std::vector<std::string>>(
      "swap_into_basis",
      "Species that should be removed from the model_definition's equilibrium species list and "
      "added to the basis.  There must be the same number of species in swap_out_of_basis and "
      "swap_into_basis.  These swaps are performed before any other computations during the "
      "initial problem setup. If this list contains more than one species, the swapping is "
      "performed one-by-one, starting with the first pair (swap_out_of_basis[0] and "
      "swap_into_basis[0]), then the next pair, etc");
  MultiMooseEnum constraint_user_meaning(
      "kg_solvent_water bulk_composition bulk_composition_with_kinetic free_concentration "
      "free_mineral activity log10activity fugacity log10fugacity");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_meaning",
      constraint_user_meaning,
      "Meanings of the numerical values given in constraint_value.  kg_solvent_water: can only be "
      "applied to H2O and units must be kg.  bulk_composition: can be applied to all non-gas "
      "species, and represents the total amount of the basis species contained as free species as "
      "well as the amount found in secondary species but not in kinetic species, and units must be "
      "moles or mass (kg, g, etc).  bulk_composition_with_kinetic: can be applied to all non-gas "
      "species, and represents the total amount of the basis species contained as free species as "
      "well as the amount found in secondary species and in kinetic species, and units must be "
      "moles or mass (kg, g, etc).  free_concentration: can be applied to all basis species that "
      "are not gas and not H2O and not mineral, and represents the total amount of the basis "
      "species existing freely (not as secondary species) within the solution, and units must be "
      "molal or mass_per_kg_solvent.  free_mineral: can be applied to all mineral basis species, "
      "and represents the total amount of the mineral existing freely (precipitated) within the "
      "solution, and units must be moles, mass or cm3.  activity and log10activity: can be applied "
      "to basis species that are not gas and not mineral and not sorbing sites, and represents the "
      "activity of the basis species (recall pH = -log10activity), and units must be "
      "dimensionless.  fugacity and log10fugacity: can be applied to gases, and units must be "
      "dimensionless");
  MultiMooseEnum constraint_unit("dimensionless moles molal kg g mg ug kg_per_kg_solvent "
                                 "g_per_kg_solvent mg_per_kg_solvent ug_per_kg_solvent cm3");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_unit",
      constraint_unit,
      "Units of the numerical values given in constraint_value.  Dimensionless: should only be "
      "used for activity or fugacity constraints.  Moles: mole number.  Molal: moles per kg "
      "solvent water.  kg: kilograms.  g: grams.  mg: milligrams.  ug: micrograms.  "
      "kg_per_kg_solvent: kilograms per kg solvent water.  g_per_kg_solvent: grams per kg solvent "
      "water.  mg_per_kg_solvent: milligrams per kg solvent water.  ug_per_kg_solvent: micrograms "
      "per kg solvent water.  cm3: cubic centimeters");
  params.addRequiredParam<std::vector<std::string>>(
      "constraint_species",
      "Names of the species that have their values fixed to constraint_value with meaning "
      "constraint_meaning.  All basis species (after swap_into_basis and swap_out_of_basis) must "
      "be provided with exactly one constraint.  These constraints are used to compute the "
      "configuration during the initial problem setup, and in time-dependent simulations they may "
      "be modified as time progresses.");
  params.addRequiredParam<std::vector<Real>>(
      "constraint_value", "Numerical value of the containts on constraint_species");
  params.addRangeCheckedParam<Real>(
      "max_ionic_strength", 3.0, "max_ionic_strength >= 0.0", "Maximum value of ionic strength");
  params.addParam<unsigned>(
      "extra_iterations_to_make_consistent",
      0,
      "Extra iterations to make the molalities, activities, etc, consistent "
      "before commencing the Newton process to find the aqueous configuration");
  params.addRequiredParam<std::string>(
      "charge_balance_species",
      "Charge balance will be enforced on this basis species.  This means that its bulk mole "
      "number may be changed from the initial value you provide in order to ensure charge "
      "neutrality.  After the initial swaps have been performed, this must be in the basis, and it "
      "must be provided with a bulk_composition constraint_meaning.");
  params.addParam<std::vector<std::string>>(
      "prevent_precipitation",
      "Mineral species in this list will be prevented from precipitating, irrespective of their "
      "saturation index (unless they are initially in the basis)");
  params.addParam<Real>(
      "abs_tol",
      1E-10,
      "If the residual of the algebraic system (measured in mol) is lower than this value, the "
      "Newton process (that finds the aqueous configuration) is deemed to have converged");
  params.addParam<Real>("rel_tol",
                        1E-200,
                        "If the residual of the algebraic system (measured in mol) is lower than "
                        "this value times the initial residual, the Newton process (that finds the "
                        "aqueous configuration) is deemed to have converged");
  params.addRangeCheckedParam<Real>("min_initial_molality",
                                    1E-20,
                                    "min_initial_molality > 0.0",
                                    "Minimum value of the initial-guess molality used in the "
                                    "Newton process to find the aqueous configuration");
  params.addParam<unsigned>(
      "max_iter",
      100,
      "Maximum number of Newton iterations allowed when finding the aqueous configuration");
  params.addParam<Real>(
      "max_initial_residual",
      1E3,
      "Attempt to alter the initial-guess molalities so that the initial residual for the Newton "
      "process (that finds the aqueous configuration) is less than this number of moles");
  params.addRangeCheckedParam<Real>(
      "swap_threshold",
      0.1,
      "swap_threshold >= 0.0",
      "If the molality of a basis species in the algebraic system falls below swap_threshold * "
      "abs_tol then it is swapped out of the basis.  The dimensions of swap_threshold are "
      "1/kg(solvent water)");
  params.addParam<unsigned>(
      "max_swaps_allowed",
      20,
      "Maximum number of swaps allowed during a single attempt at finding the aqueous "
      "configuration.  Usually only a handful of swaps are used: this parameter prevents endless "
      "cyclic swapping that prevents the algorithm from progressing");
  params.addParam<unsigned>(
      "ramp_max_ionic_strength_initial",
      20,
      "The number of iterations over which to progressively increase the maximum ionic strength "
      "(from zero to max_ionic_strength) during the initial equilibration.  Increasing this can "
      "help in convergence of the Newton process, at the cost of spending more time finding the "
      "aqueous configuration.");
  params.addParam<bool>(
      "ionic_str_using_basis_only",
      false,
      "If set to true, ionic strength and stoichiometric ionic strength will be computed using "
      "only the basis molalities, ignoring molalities of equilibrium species.  Since basis "
      "molality is usually greater than equilibrium molality, and the whole Debye-Huckel concept "
      "of activity coefficients depending on ionic strength is only approximate in practice, "
      "setting this parameter true often results in a reasonable approximation.  It can aid in "
      "convergence since it eliminates problems associated with unphysical huge equilibrium "
      "molalities that can occur during Newton iteration to the solution");
  params.addParam<bool>("stoichiometric_ionic_str_using_Cl_only",
                        false,
                        "If set to true, the stoichiometric ionic strength will be set equal to "
                        "Cl- molality (or max_ionic_strength if the Cl- molality is too "
                        "big).  This flag overrides ionic_str_using_basis_molality_only");
  return params;
}

InputParameters
GeochemistryReactorBase::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params += GeochemistryReactorBase::sharedParams();

  params.addRequiredParam<UserObjectName>(
      "model_definition", "The name of the GeochemicalModelDefinition user object.");
  params.addRangeCheckedParam<Real>(
      "stoichiometry_tolerance",
      1E-6,
      "stoichiometry_tolerance >= 0.0",
      "Swapping involves inverting matrices via a singular value decomposition. During this "
      "process: (1) if abs(singular value) < stoi_tol * L1norm(singular values), then the "
      "matrix is deemed singular (so the basis swap is deemed invalid); (2) if abs(any "
      "stoichiometric coefficient) < stoi_tol then it is set to zero.");
  params.addClassDescription("Base class for UserObject to solve geochemistry reactions");
  return params;
}

GeochemistryReactorBase::GeochemistryReactorBase(const InputParameters & parameters)
  : NodalUserObject(parameters),
    _num_my_nodes(_subproblem.mesh().getMesh().n_local_nodes()),
    _mgd(getUserObject<GeochemicalModelDefinition>("model_definition").getDatabase()),
    _pgs(getUserObject<GeochemicalModelDefinition>("model_definition")
             .getPertinentGeochemicalSystem()),
    _num_basis(_mgd.basis_species_name.size()),
    _num_eqm(_mgd.eqm_species_name.size()),
    _initial_max_ionic_str(getParam<Real>("max_ionic_strength") /
                           (1.0 + getParam<unsigned>("ramp_max_ionic_strength_initial"))),
    _is(_initial_max_ionic_str,
        _initial_max_ionic_str,
        getParam<bool>("ionic_str_using_basis_only"),
        getParam<bool>("stoichiometric_ionic_str_using_Cl_only")),
    _gac(_is,
         getUserObject<GeochemicalModelDefinition>("model_definition").getOriginalFullDatabase()),
    _max_swaps_allowed(getParam<unsigned>("max_swaps_allowed")),
    _swapper(_num_basis, getParam<Real>("stoichiometry_tolerance")),
    _small_molality(getParam<Real>("swap_threshold") * getParam<Real>("abs_tol")),
    _solver_output(_num_my_nodes),
    _tot_iter(_num_my_nodes, 0),
    _abs_residual(_num_my_nodes, 0.0)
{
}

void
GeochemistryReactorBase::initialize()
{
}
void
GeochemistryReactorBase::finalize()
{
}
void
GeochemistryReactorBase::threadJoin(const UserObject & /*uo*/)
{
}

void
GeochemistryReactorBase::execute()
{
}
