//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermochimicaNodalData.h"
#include "ChemicalCompositionAction.h"
#include "ThermochimicaUtils.h"
#include "ActionWarehouse.h"
#include "libmesh/int_range.h"

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#include "checkUnits.h"
#endif

registerMooseObject("ChemicalReactionsApp", ThermochimicaNodalData);

InputParameters
ThermochimicaNodalData::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  ThermochimicaUtils::addClassDescription(
      params, "Provides access to Thermochimica-calculated data at nodes.");

  params.addRequiredCoupledVar("elements", "Amounts of elements");
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addCoupledVar("pressure", 1.0, "Pressure");

  params.addParam<bool>("reinit_requested", true, "Should Thermochimica use re-initialization?");

  params.addCoupledVar("output_element_potentials", "Chemical potentials of elements");
  params.addCoupledVar("output_phases", "Amounts of phases to be output");
  params.addCoupledVar("output_species", "Amounts of species to be output");
  params.addCoupledVar("output_vapor_pressures", "Vapour pressures of species to be output");
  params.addCoupledVar("output_element_phases",
                       "Elements whose molar amounts in specific phases are requested");

  params.addPrivateParam<ChemicalCompositionAction *>("_chemical_composition_action");
  return params;
}

ThermochimicaNodalData::ThermochimicaNodalData(const InputParameters & parameters)
  : NodalUserObject(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
    _reinit_requested(getParam<bool>("reinit_requested")),
    _n_phases(coupledComponents("output_phases")),
    _n_species(coupledComponents("output_species")),
    _n_elements(coupledComponents("elements")),
    _n_vapor_species(coupledComponents("output_vapor_pressures")),
    _n_phase_elements(coupledComponents("output_element_phases")),
    _n_potentials(coupledComponents("output_element_potentials")),
    _el(_n_elements),
    _action(*parameters.getCheckedPointerParam<ChemicalCompositionAction *>(
        "_chemical_composition_action")),
    _el_ids(_action.elementIDs()),
    _ph_names(_action.phases()),
    _element_potentials(_action.elementPotentials()),
    _species_phase_pairs(_action.speciesPhasePairs()),
    _vapor_phase_pairs(_action.vaporPhasePairs()),
    _phase_element_pairs(_action.phaseElementPairs()),
    _output_element_potentials(isCoupled("output_element_potentials")),
    _output_vapor_pressures(isCoupled("output_vapor_pressures")),
    _output_element_phases(isCoupled("output_element_phases")),
    _ph(_n_phases),
    _sp(_n_species),
    _vp(_n_vapor_species),
    _el_pot(_n_potentials),
    _el_ph(_n_phase_elements)
{
  ThermochimicaUtils::checkLibraryAvailability(*this);

  if (n_threads() > 1)
    mooseError("Thermochimica does not support multi-threaded runs.");

  if (_el_ids.size() != _n_elements)
    mooseError("Element IDs size does not match number of elements.");
  for (const auto i : make_range(_n_elements))
    _el[i] = &coupledValue("elements", i);

  if (isParamValid("output_phases"))
  {
    if (_ph_names.size() != _n_phases)
      mooseError("Phase names vector size does not match number of phases.");

    for (const auto i : make_range(_n_phases))
      _ph[i] = &writableVariable("output_phases", i);
  }

  if (isParamValid("output_species"))
  {
    if (_species_phase_pairs.size() != _n_species)
      mooseError("Species name vector size does not match number of output species.");

    for (const auto i : make_range(_n_species))
      _sp[i] = &writableVariable("output_species", i);
  }

  if (isParamValid("output_vapor_pressures"))
  {
    if (_vapor_phase_pairs.size() != _n_vapor_species)
      mooseError("Vapor species name vector size does not match number of output vapor species.");

    for (const auto i : make_range(_n_vapor_species))
      _vp[i] = &writableVariable("output_vapor_pressures", i);
  }

  if (isParamValid("output_element_phases"))
  {
    if (_phase_element_pairs.size() != _n_phase_elements)
      mooseError("Element phase vector size does not match number of output elements in phases");

    for (const auto i : make_range(_n_phase_elements))
      _el_ph[i] = &writableVariable("output_element_phases", i);
  }

  if (isParamValid("output_element_potentials"))
  {
    if (_element_potentials.size() != _n_potentials)
      mooseError("Element potentials vector size does not match number of element potentials "
                 "specified for output.");

    for (const auto i : make_range(_n_potentials))
      _el_pot[i] = &writableVariable("output_element_potentials", i);
  }
}

void
ThermochimicaNodalData::initialize()
{
}

void
ThermochimicaNodalData::execute()
{
#ifdef THERMOCHIMICA_ENABLED
  auto temperature = _temperature[_qp];
  auto pressure = _pressure[_qp];

  // Set temperature and pressure for thermochemistry solver
  Thermochimica::setTemperaturePressure(temperature, pressure);

  // Reset all element masses to 0
  Thermochimica::setElementMass(0, 0.0);

  // Set element masses
  for (const auto i : make_range(_n_elements))
    Thermochimica::setElementMass(_el_ids[i], (*_el[i])[_qp]);

  // Optionally ask for a re-initialization (if reinit_requested == true)
  reinitDataMooseToTc();

  // Calculate thermochemical equilibrium
  Thermochimica::thermochimica();

  // fetch data for the current node
  auto & d = _data[_current_node->id()];

  // Check for error status
  auto idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
    mooseError("Thermochimica error ", idbg);
  else
  {
    // Save data for future reinits
    reinitDataMooseFromTc();

    // Get requested phase indices if phase concentration output was requested
    // i.e. if output_phases is coupled
    for (const auto i : make_range(_n_phases))
    {
      // Is this maybe constant? No it isn't for now
      auto [index, idbg] = Thermochimica::getPhaseIndex(_ph_names[i]);
      if (idbg != 0)
        mooseError("Failed to get index of phase '", _ph_names[i], "'");
      // Convert from 1-based (fortran) to 0-based (c++) indexing
      if (index - 1 < 0)
        _ph[i]->setNodalValue(0.0, _qp);
      else
        _ph[i]->setNodalValue(d._moles_phase[index - 1], _qp);
    }

    auto db_phases = Thermochimica::getPhaseNamesSystem();
    for (const auto i : make_range(_n_species))
    {
      auto ph_index = std::distance(
          db_phases.begin(),
          std::find(db_phases.begin(), db_phases.end(), _species_phase_pairs[i].first));

      auto [fraction, idbg] =
          // can we somehow use IDs instead of strings here?
          Thermochimica::isPhaseMQM(ph_index)
              ? Thermochimica::getMqmqaPairMolFraction(_species_phase_pairs[i].first,
                                                       _species_phase_pairs[i].second)
              : Thermochimica::getOutputMolSpeciesPhase(_species_phase_pairs[i].first,
                                                        _species_phase_pairs[i].second);

      if (idbg == 0)
        _sp[i]->setNodalValue(fraction, _qp);
      else if (idbg == 1)
        _sp[i]->setNodalValue(0.0, _qp);
#ifndef NDEBUG
      else
        mooseError("Failed to get phase speciation for phase '",
                   _species_phase_pairs[i].first,
                   "' and species '",
                   _species_phase_pairs[i].second,
                   "'. Thermochimica returned ",
                   idbg);
#endif
    }

    if (_output_element_potentials)
      for (const auto i : index_range(_element_potentials))
      {
        auto [potential, idbg] = Thermochimica::getOutputChemPot(_element_potentials[i]);

        if (idbg == 0)
          _el_pot[i]->setNodalValue(potential, _qp);
        else if (idbg == 1)
          // element not present, just leave this at 0 for now
          _el_pot[i]->setNodalValue(0.0, _qp);
        else if (idbg == -1)
          Moose::out << "getoutputchempot " << idbg << "\n";
      }

    if (_output_vapor_pressures)
      for (const auto i : make_range(_n_vapor_species))
      {
        auto [fraction, moles, idbg] =
            Thermochimica::getOutputMolSpecies(_vapor_phase_pairs[i].second);
        libmesh_ignore(moles);

        if (idbg == 0)
          _vp[i]->setNodalValue(fraction * pressure, _qp);
        else if (idbg == 1)
          _vp[i]->setNodalValue(0.0, _qp);
#ifndef NDEBUG
        else
          mooseError("Failed to get vapor pressure for phase '",
                     _vapor_phase_pairs[i].first,
                     "' and species '",
                     _vapor_phase_pairs[i].second,
                     "'. Thermochimica returned ",
                     idbg);
#endif
      }

    if (_output_element_phases)
      for (const auto i : make_range(_n_phase_elements))
      {
        auto [moles, idbg] = Thermochimica::getElementMolesInPhase(_phase_element_pairs[i].second,
                                                                   _phase_element_pairs[i].first);

        if (idbg == 0)
          _el_ph[i]->setNodalValue(moles, _qp);
        else if (idbg == 1)
          _el_ph[i]->setNodalValue(0.0, _qp);
#ifndef NDEBUG
        else
          mooseError("Failed to get moles of element '",
                     _phase_element_pairs[i].second,
                     "' in phase '",
                     _phase_element_pairs[i].first,
                     "'. Thermochimica returned ",
                     idbg);
#endif
      }
  }
#endif
}

void
ThermochimicaNodalData::reinitDataMooseFromTc()
{
#ifdef THERMOCHIMICA_ENABLED
  auto & d = _data[_current_node->id()];

  if (_reinit_requested)
  {
    Thermochimica::saveReinitData();
    auto data = Thermochimica::getReinitData();
    d._assemblage = std::move(data.assemblage);
    d._moles_phase = std::move(data.molesPhase);
    d._element_potential = std::move(data.elementPotential);
    d._chemical_potential = std::move(data.chemicalPotential);
    d._mol_fraction = std::move(data.moleFraction);
    d._elements_used = std::move(data.elementsUsed);
    d._reinit_available = data.reinitAvailable;
  }
  else
  {
    // If phase concentration data output has been requested, _moles_phase is required even if other
    // re-initialization data is not
    if (_n_phases > 0)
      d._moles_phase = Thermochimica::getMolesPhase();

    // If element chemical potential data output has been requested, _element_potential is required
    // even if other re-initialization data is not
    if (_output_element_potentials)
      d._element_potential = Thermochimica::getAllElementPotential();
  }
#endif
}

void
ThermochimicaNodalData::reinitDataMooseToTc()
{
#ifdef THERMOCHIMICA_ENABLED
  // Tell Thermochimica whether a re-initialization is requested for this calculation
  Thermochimica::setReinitRequested(_reinit_requested);

  // If we have re-initialization data and want a re-initialization, then
  // load data into Thermochimica
  auto it = _data.find(_current_node->id());
  if (it != _data.end() && _reinit_requested)
  {
    auto & d = it->second;
    if (d._reinit_available)
    {
      Thermochimica::resetReinit();
      Thermochimica::reinitData data;
      data.assemblage = d._assemblage;
      data.molesPhase = d._moles_phase;
      data.elementPotential = d._element_potential;
      data.chemicalPotential = d._chemical_potential;
      data.moleFraction = d._mol_fraction;
      data.elementsUsed = d._elements_used;
      Thermochimica::setReinitData(data);
    }
  }
#endif
}

const ThermochimicaNodalData::Data &
ThermochimicaNodalData::getNodalData(dof_id_type node_id) const
{
  const auto it = _data.find(node_id);
  if (it == _data.end())
    mooseError("Unable to look up data for node ", node_id);
  return it->second;
}
