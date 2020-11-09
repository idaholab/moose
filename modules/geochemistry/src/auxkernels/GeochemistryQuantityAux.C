//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryQuantityAux.h"

registerMooseObject("GeochemistryApp", GeochemistryQuantityAux);

InputParameters
GeochemistryQuantityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Extracts information from the Reactor and records it in the AuxVariable");
  params.addRequiredParam<std::string>("species", "Species name");
  MooseEnum quantity_choice("molal mg_per_kg free_mg free_cm3 neglog10a activity bulk_moles "
                            "surface_charge surface_potential temperature kinetic_moles "
                            "kinetic_additions moles_dumped transported_moles_in_original_basis",
                            "molal");
  params.addParam<MooseEnum>(
      "quantity",
      quantity_choice,
      "Type of quantity to output.  These are available for non-kinetic species: activity (which "
      "equals "
      "fugacity for gases); bulk moles (this will be zero if the species is not in the basis); "
      "neglog10a = -log10(activity); transported_moles_in_original_basis (will throw an error if "
      "species is not in original basis).  These are available only for non-kinetic non-minerals: "
      "molal = "
      "mol(species)/kg(solvent_water); mg_per_kg = mg(species)/kg(solvent_water).  These are "
      "available only for minerals: "
      "free_mg = free mg; free_cm3 = free cubic-centimeters; moles_dumped = moles dumped when "
      "special dump and flow-through modes are active.  These are available for minerals "
      "that host sorbing sites: surface_charge (C/m^2); surface_potential (V).  These are "
      "available for kinetic species: kinetic_moles; kinetic_additions (-dt * rate = mole "
      "increment in kinetic species for this timestep).  If "
      "quantity=temperature, then the 'species' is ignored and the AuxVariable will record the "
      "aqueous solution temperature in degC");
  params.addRequiredParam<UserObjectName>("reactor",
                                          "The name of the Geochemistry*Reactor user object.");
  params.declareControllable("species");
  return params;
}

GeochemistryQuantityAux::GeochemistryQuantityAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _species(getParam<std::string>("species")),
    _reactor(getUserObject<GeochemistryReactorBase>("reactor")),
    _quantity_choice(getParam<MooseEnum>("quantity").getEnum<QuantityChoiceEnum>()),
    _surface_sorption_mineral_index(0)
{
  const ModelGeochemicalDatabase & mgd =
      _reactor.getPertinentGeochemicalSystem().modelGeochemicalDatabase();
  if (!(mgd.basis_species_index.count(_species) == 1 ||
        mgd.eqm_species_index.count(_species) == 1 || mgd.kin_species_index.count(_species) == 1))
    paramError(
        "species",
        _species,
        " does not appear in the model's geochemical system either as a basis or equilibrium "
        "or kinetic species, but you requested an Aux involving it");

  bool is_mineral = false;
  if (mgd.basis_species_index.count(_species) == 1)
    is_mineral = mgd.basis_species_mineral[mgd.basis_species_index.at(_species)];
  else if (mgd.eqm_species_index.count(_species) == 1)
    is_mineral = mgd.eqm_species_mineral[mgd.eqm_species_index.at(_species)];
  else
    is_mineral = mgd.kin_species_mineral[mgd.kin_species_index.at(_species)];
  bool is_kinetic = (mgd.kin_species_index.count(_species) == 1);

  if (!is_mineral && (_quantity_choice == QuantityChoiceEnum::FREE_CM3 ||
                      _quantity_choice == QuantityChoiceEnum::FREE_MG ||
                      _quantity_choice == QuantityChoiceEnum::SURFACE_CHARGE ||
                      _quantity_choice == QuantityChoiceEnum::SURFACE_POTENTIAL ||
                      _quantity_choice == QuantityChoiceEnum::MOLES_DUMPED))
    paramError(
        "quantity",
        "the free_mg, free_cm3, moles_dumped and surface-related quantities are only available for "
        "mineral species");
  if ((is_mineral || is_kinetic) && (_quantity_choice == QuantityChoiceEnum::MOLAL ||
                                     _quantity_choice == QuantityChoiceEnum::MG_PER_KG))
    paramError("quantity",
               "the molal and mg_per_kg quantities are only available for "
               "non-kinetic, non-mineral species");
  if (_quantity_choice == QuantityChoiceEnum::SURFACE_CHARGE ||
      _quantity_choice == QuantityChoiceEnum::SURFACE_POTENTIAL)
  {
    _surface_sorption_mineral_index = std::distance(
        mgd.surface_sorption_name.begin(),
        std::find(mgd.surface_sorption_name.begin(), mgd.surface_sorption_name.end(), _species));
    if (_surface_sorption_mineral_index >= mgd.surface_sorption_name.size())
      paramError("species",
                 "cannot record surface charge or surface potential for a species that is not "
                 "involved in surface sorption");
  }
  if (!is_kinetic && (_quantity_choice == QuantityChoiceEnum::KINETIC_MOLES ||
                      _quantity_choice == QuantityChoiceEnum::KINETIC_ADDITIONS))
    paramError("quantity",
               "the kinetic_moles and kinetic_additions quantities are only available for kinetic "
               "species");
  if (is_kinetic && (_quantity_choice == QuantityChoiceEnum::NEGLOG10A ||
                     _quantity_choice == QuantityChoiceEnum::ACTIVITY ||
                     _quantity_choice == QuantityChoiceEnum::BULK_MOLES))
    paramError("species", "cannot record activity, neglog10a or bulk_moles for a kinetic species");
  if (_quantity_choice == QuantityChoiceEnum::TRANSPORTED_MOLES_IN_ORIGINAL_BASIS)
    _reactor.getPertinentGeochemicalSystem().getIndexOfOriginalBasisSpecies(
        _species); // will throw error if species not in original basis
}

Real
GeochemistryQuantityAux::computeValue()
{
  Real ret = 0.0;
  const GeochemicalSystem & egs = _reactor.getGeochemicalSystem(_current_node->id());
  const ModelGeochemicalDatabase & mgd = egs.getModelGeochemicalDatabase();
  const PertinentGeochemicalSystem & pgs = _reactor.getPertinentGeochemicalSystem();
  if (_quantity_choice == QuantityChoiceEnum::SURFACE_CHARGE)
    ret = egs.getSurfaceCharge(_surface_sorption_mineral_index);
  else if (_quantity_choice == QuantityChoiceEnum::SURFACE_POTENTIAL)
    ret = egs.getSurfacePotential(_surface_sorption_mineral_index);
  else if (_quantity_choice == QuantityChoiceEnum::TEMPERATURE)
    ret = egs.getTemperature();
  else if (_quantity_choice == QuantityChoiceEnum::MOLES_DUMPED)
    ret = _reactor.getMolesDumped(_current_node->id(), _species);
  else if (_quantity_choice == QuantityChoiceEnum::TRANSPORTED_MOLES_IN_ORIGINAL_BASIS)
    ret = egs.getTransportedBulkInOriginalBasis()(pgs.getIndexOfOriginalBasisSpecies(_species));
  else
  {
    if (mgd.basis_species_index.count(_species) == 1)
    {
      const unsigned basis_ind = mgd.basis_species_index.at(_species);
      switch (_quantity_choice)
      {
        case QuantityChoiceEnum::MG_PER_KG:
          ret = egs.getSolventMassAndFreeMolalityAndMineralMoles()[basis_ind] *
                mgd.basis_species_molecular_weight[basis_ind] * 1000.0;
          break;
        case QuantityChoiceEnum::FREE_MG:
          ret = egs.getSolventMassAndFreeMolalityAndMineralMoles()[basis_ind] *
                mgd.basis_species_molecular_weight[basis_ind] * 1000.0;
          break;
        case QuantityChoiceEnum::FREE_CM3:
          ret = egs.getSolventMassAndFreeMolalityAndMineralMoles()[basis_ind] *
                mgd.basis_species_molecular_volume[basis_ind];
          break;
        case QuantityChoiceEnum::NEGLOG10A:
          ret = -std::log10(egs.getBasisActivity(basis_ind));
          break;
        case QuantityChoiceEnum::ACTIVITY:
          ret = egs.getBasisActivity(basis_ind);
          break;
        case QuantityChoiceEnum::BULK_MOLES:
          ret = egs.getBulkMolesOld()[basis_ind];
          break;
        default:
          ret = egs.getSolventMassAndFreeMolalityAndMineralMoles()[basis_ind];
      }
    }
    else if (mgd.eqm_species_index.count(_species) == 1)
    {
      const unsigned eqm_ind = mgd.eqm_species_index.at(_species);
      switch (_quantity_choice)
      {
        case QuantityChoiceEnum::MG_PER_KG:
          ret = egs.getEquilibriumMolality(eqm_ind) * mgd.eqm_species_molecular_weight[eqm_ind] *
                1000.0;
          break;
        case QuantityChoiceEnum::FREE_CM3:
          ret = egs.getEquilibriumMolality(eqm_ind) * mgd.eqm_species_molecular_volume[eqm_ind];
          break;
        case QuantityChoiceEnum::FREE_MG:
          ret = egs.getEquilibriumMolality(eqm_ind) * mgd.eqm_species_molecular_weight[eqm_ind] *
                1000.0;
          break;
        case QuantityChoiceEnum::NEGLOG10A:
          ret = -std::log10(egs.getEquilibriumActivity(eqm_ind));
          break;
        case QuantityChoiceEnum::ACTIVITY:
          ret = egs.getEquilibriumActivity(eqm_ind);
          break;
        case QuantityChoiceEnum::BULK_MOLES:
          ret = 0.0;
          break;
        default:
          ret = egs.getEquilibriumMolality(eqm_ind);
      }
    }
    else
    {
      const unsigned kin_ind = mgd.kin_species_index.at(_species);
      switch (_quantity_choice)
      {
        case QuantityChoiceEnum::FREE_CM3:
          ret = egs.getKineticMoles(kin_ind) * mgd.kin_species_molecular_volume[kin_ind];
          break;
        case QuantityChoiceEnum::FREE_MG:
          ret = egs.getKineticMoles(kin_ind) * mgd.kin_species_molecular_weight[kin_ind] * 1000.0;
          break;
        case QuantityChoiceEnum::KINETIC_ADDITIONS:
          ret = _reactor.getMoleAdditions(_current_node->id())(kin_ind + egs.getNumInBasis());
          break;
        default:
          ret = egs.getKineticMoles(kin_ind);
      }
    }
  }
  return ret;
}
