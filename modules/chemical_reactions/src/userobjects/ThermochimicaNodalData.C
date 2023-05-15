//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermochimicaNodalData.h"
#include "ThermochimicaUtils.h"
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
  params.addCoupledVar("element_potentials", "Chemical potentials of elements");

  params.addCoupledVar("pressure", 1.0, "Pressure");
  params.addRequiredCoupledVar("temperature", "Coupled temperature");

  params.addParam<bool>("reinit_requested", true, "Should Thermochimica use re-initialization?");
  params.addCoupledVar("output_phases", "Amounts of phases to be output");
  params.addCoupledVar("output_species", "Amounts of species to be output");
  params.addCoupledVar("output_vapor_pressures", "Vapour pressures of species to be output");
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
    _el(_n_elements),
    _el_name(_n_elements),
    _ph_name(_n_phases),
    _sp_phase_name(_n_species),
    _sp_species_name(_n_species),
    _vapor_phase_name(_n_vapor_species),
    _vapor_species_name(_n_vapor_species),
    _output_element_potential(isCoupled("element_potentials")),
    _output_vapor_pressures(isCoupled("output_vapor_pressures"))
{
  ThermochimicaUtils::checkLibraryAvailability(*this);

  if (n_threads() > 1)
    mooseError("Thermochimica does not support multi-threaded runs.");
  if (coupledComponents("element_potentials") != _n_elements)
    paramError("element_potentials", "Specify one element potential for each entry in `elements`");

  for (const auto i : make_range(_n_elements))
  {
    _el[i] = &coupledValue("elements", i);
    _el_name[i] = getVar("elements", i)->name();
#ifdef THERMOCHIMICA_ENABLED
    // check if the element symbol is valid
    _el_id.resize(_n_elements);
    _el_id[i] = Thermochimica::atomicNumber(_el_name[i]);
#endif
  }

#ifdef THERMOCHIMICA_ENABLED
  {
    // Get thermodynamic database information to compare with input files.
    _db_num_phases = Thermochimica::getNumberPhasesDatabase();
    _db_phase_names = Thermochimica::getPhaseNamesDatabase();
    _db_species_names = Thermochimica::getSpeciesDatabase();

    for (const auto i : make_range(_n_phases))
    {
      _ph_name[i] = getVar("output_phases", i)->name();
      if (std::find(_db_phase_names.begin(), _db_phase_names.end(), _ph_name[i]) ==
          _db_phase_names.end())
        paramError("output_phases", "Phase '", _ph_name[i], "' was not found in the simulation.");
    }

    for (const auto i : make_range(_n_species))
    {
      auto species_var_name = getVar("output_species", i)->name();
      auto colon = species_var_name.find_last_of(':');
      if (colon == std::string::npos)
        paramError("output_species", "No ':' separator found in variable '", species_var_name, "'");
      _sp_phase_name[i] = species_var_name.substr(0, colon);
      if (std::find(_db_phase_names.begin(), _db_phase_names.end(), _sp_phase_name[i]) ==
          _db_phase_names.end())
        paramError("output_species",
                   "Phase name '",
                   _sp_phase_name[i],
                   "' of output species '",
                   species_var_name,
                   "' was not found in the simulation.");
      _sp_species_name[i] = species_var_name.substr(colon + 1);

      auto ph_index = std::distance(
          _db_phase_names.begin(),
          std::find(_db_phase_names.begin(), _db_phase_names.end(), _sp_phase_name[i]));
      if (std::find(_db_species_names[ph_index].begin(),
                    _db_species_names[ph_index].end(),
                    _sp_species_name[i]) == _db_species_names[ph_index].end())
        paramError("output_species",
                   "Species '",
                   _sp_species_name[i],
                   "' was not found in the simulation.");
    }

    if (_output_element_potential)
    {
      _element_potentials.resize(_n_elements);
      for (const auto i : make_range(_n_elements))
      {
        auto element_var_name = getVar("element_potentials", i)->name();
        auto colon = element_var_name.find_last_of(':');
        if (colon == std::string::npos)
          paramError(
              "element_potentials", "No ':' separator found in variable '", element_var_name, "'");
        _element_potentials[i] = element_var_name.substr(colon + 1);
        if (std::find(_el_name.begin(), _el_name.end(), _element_potentials[i]) == _el_name.end())
          paramError("element_potentials",
                     "Element '",
                     _element_potentials[i],
                     "' was not found in the simulation.");
      }
    }

    if (_output_vapor_pressures)
    {
      if (!(Thermochimica::getNumberSpeciesDatabase()[0] > 0))
        paramError("output_vapor_pressures",
                   "Vapor pressures requested but database contains no gas phase.");

      for (const auto i : make_range(_n_vapor_species))
      {
        auto vapor_species_name = getVar("output_vapor_pressures", i)->name();
        auto colon = vapor_species_name.find_last_of(':');
        if (colon == std::string::npos)
          paramError("output_vapor_pressures",
                     "No ':' separator found in variable '",
                     vapor_species_name,
                     "'");
        _vapor_phase_name[i] = vapor_species_name.substr(0, colon);
        if (_vapor_phase_name[i] != _db_phase_names[0])
          paramError("output_vapor_pressures",
                     "Phase '",
                     _vapor_phase_name[i],
                     "' of vapor species '",
                     vapor_species_name,
                     "' is not a gas phase. Cannot calculate vapor pressure.");
        _vapor_species_name[i] = vapor_species_name.substr(colon + 1);
        if (std::find(_db_species_names[0].begin(),
                      _db_species_names[0].end(),
                      _vapor_species_name[i]) == _db_species_names[0].end())
          paramError("output_vapor_pressures",
                     "Species '",
                     _vapor_species_name[i],
                     "' was not found in the gas phase.");
      }
    }
    // get phase name && phase name should be that of gas phase && gas phase should be in the
    // database gas phase in the database : get number of species in solution phase 0 and if that is
    // greater than 0 then we definitely have a gas phase because that is how factsage format is.
    // Then we can compare to the name of the first phase in database. If the phase name is wrong
    // then there is an issue. If the phase name is correct then check the database
  }
#endif
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

  Thermochimica::setElementMass(0, 0.0); // Reset all element masses to 0

  // Set element masses
  for (const auto i : make_range(_n_elements))
    Thermochimica::setElementMass(_el_id[i], (*_el[i])[_qp]);

  // Optionally ask for a re-initialization (if reinit_requested == true)
  reinitDataMooseToTc();

  // Calculate thermochemical equilibrium
  Thermochimica::thermochimica();

  // fetch data for the current node
  auto & d = _data[_current_node->id()];

  // Check for error status
  auto idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
  {
    // Thermochimica::printState();
    mooseError("Thermochimica error ", idbg);
  }
  else
  {
    // Get requested phase indices if phase concentration output was requested
    // i.e. if output_phases is coupled
    d._phase_indices.resize(_n_phases);
    for (const auto i : make_range(_n_phases))
    {
      // Is this maybe constant? No it isn't for now
      auto [index, idbg] = Thermochimica::getPhaseIndex(_ph_name[i]);
      if (idbg != 0)
        mooseError("Failed to get index of phase '", _ph_name[i], "'");
      // Convert from 1-based (fortran) to 0-based (c++) indexing
      d._phase_indices[i] = index - 1;
    }

    // Save data for future reinits
    reinitDataMooseFromTc();

    d._species_fractions.resize(_n_species);
    for (const auto i : make_range(_n_species))
    {
      auto [fraction, idbg] =
          // can we somehow use IDs instead of strings here?
          Thermochimica::getOutputMolSpeciesPhase(_sp_phase_name[i], _sp_species_name[i]);

      if (idbg == 0)
        d._species_fractions[i] = fraction;
      else if (idbg == 1)
        d._species_fractions[i] = 0;
      else
        mooseError("Failed to get phase speciation for phase '",
                   _sp_phase_name[i],
                   "' and species '",
                   _sp_species_name[i],
                   "'. Thermochimica returned ",
                   idbg);
    }

    if (_output_element_potential)
    {
      d._element_potential_for_output.resize(_element_potentials.size());
      for (const auto i : index_range(_element_potentials))
      {
        auto [potential, idbg] = Thermochimica::getOutputChemPot(_element_potentials[i]);

        if (idbg == 0)
          d._element_potential_for_output[i] = potential;
        else if (idbg == 1)
          // element not present, just leave this at 0 for now
          d._element_potential_for_output[i] = 0.0;
        else if (idbg == -1)
          Moose::out << "getoutputchempot " << idbg << "\n";
      }
    }

    if (_output_vapor_pressures)
    {
      d._vapor_pressures.resize(_n_vapor_species);
      for (const auto i : make_range(_n_vapor_species))
      {
        auto [fraction, moles, idbg] = Thermochimica::getOutputMolSpecies(_vapor_species_name[i]);
        libmesh_ignore(moles);

        if (idbg == 0)
          d._vapor_pressures[i] = fraction * pressure;
        else if (idbg == 1)
          d._vapor_pressures[i] = 0;
        else
          mooseError("Failed to get vapor pressure for phase '",
                     _vapor_phase_name[i],
                     "' and species '",
                     _vapor_species_name[i],
                     "'. Thermochimica returned ",
                     idbg);
      }
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
    if (_output_element_potential)
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
