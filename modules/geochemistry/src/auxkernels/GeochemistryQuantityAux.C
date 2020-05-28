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
  params.addClassDescription("TODO");
  params.addRequiredParam<std::string>("species", "Species name");
  MooseEnum quantity_choice("molal mg_per_kg free_mg free_cm3 neglog10a activity bulk_moles "
                            "surface_charge surface_potential temperature",
                            "molal");
  params.addParam<MooseEnum>(
      "quantity",
      quantity_choice,
      "Type of quantity to output.  These are available for all species: activity (which equals "
      "fugacity for gases); bulk moles "
      "(this will be zero if the species is not in the basis).  These are available only for "
      "non-minerals: molal = "
      "mol(species)/kg(solvent_water); mg_per_kg "
      "= mg(species)/kg(solvent_water); neglog10a = -log10(activity).  These "
      "are available only for minerals: "
      "free_mg = free mg; free_cm3 = free cubic-centimeters.  These are available for minerals "
      "that host sorbing sites: surface_charge (C/m^2); surface_potential (V).  If "
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
      _reactor.getEquilibriumGeochemicalSystem(0).getModelGeochemicalDatabase();
  if (!(mgd.basis_species_index.count(_species) == 1 || mgd.eqm_species_index.count(_species) == 1))
    paramError(
        "species",
        _species,
        " does not appear in the model's geochemical system either as a basis or equilibrium "
        "species, but you requested an Aux involving it");
  bool is_mineral = false;
  if (mgd.basis_species_index.count(_species) == 1)
    is_mineral = mgd.basis_species_mineral[mgd.basis_species_index.at(_species)];
  else
    is_mineral = mgd.eqm_species_mineral[mgd.eqm_species_index.at(_species)];
  if (!is_mineral && (_quantity_choice == QuantityChoiceEnum::FREE_CM3 ||
                      _quantity_choice == QuantityChoiceEnum::FREE_MG ||
                      _quantity_choice == QuantityChoiceEnum::SURFACE_CHARGE ||
                      _quantity_choice == QuantityChoiceEnum::SURFACE_POTENTIAL))
    paramError("quantity",
               "the free_mg, free_cm3 and surface-related quantities are only available for "
               "mineral species");
  if (is_mineral && (_quantity_choice == QuantityChoiceEnum::MOLAL ||
                     _quantity_choice == QuantityChoiceEnum::MG_PER_KG ||
                     _quantity_choice == QuantityChoiceEnum::NEGLOG10A))
    paramError("quantity",
               "the molal and mg_per_kg and neglog10a quantities are only available for "
               "non-mineral species");
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
}

Real
GeochemistryQuantityAux::computeValue()
{
  Real ret = 0.0;
  const EquilibriumGeochemicalSystem & egs =
      _reactor.getEquilibriumGeochemicalSystem(_current_node->id());
  const ModelGeochemicalDatabase & mgd = egs.getModelGeochemicalDatabase();
  if (_quantity_choice == QuantityChoiceEnum::SURFACE_CHARGE)
    ret = egs.getSurfaceCharge(_surface_sorption_mineral_index);
  else if (_quantity_choice == QuantityChoiceEnum::SURFACE_POTENTIAL)
    ret = egs.getSurfacePotential(_surface_sorption_mineral_index);
  else if (_quantity_choice == QuantityChoiceEnum::TEMPERATURE)
    ret = egs.getTemperature();
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
          ret = egs.getBulkMoles()[basis_ind];
          break;
        default:
          ret = egs.getSolventMassAndFreeMolalityAndMineralMoles()[basis_ind];
      }
    }
    else
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
          ret = -std::log10(egs.getEquilibriumMolality(eqm_ind) *
                            egs.getEquilibriumActivityCoefficient(eqm_ind));
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
  }
  return ret;
}
