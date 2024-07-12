//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermochimicaData.h"
#include "ChemicalCompositionAction.h"
#include "ThermochimicaUtils.h"
#include "ActionWarehouse.h"
#include "libmesh/int_range.h"

#include <sys/mman.h>   // for mmap
#include <unistd.h>     // for fork
#include <sys/socket.h> // for socketpair
#include <csignal>      // for kill

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#include "checkUnits.h"
#endif

registerMooseObject("ChemicalReactionsApp", ThermochimicaNodalData);
registerMooseObject("ChemicalReactionsApp", ThermochimicaElementData);

template <bool is_nodal>
InputParameters
ThermochimicaDataBase<is_nodal>::validParams()
{
  InputParameters params = ThermochimicaDataBaseParent<is_nodal>::validParams();

  if constexpr (is_nodal)
    ThermochimicaUtils::addClassDescription(
        params, "Provides access to Thermochimica-calculated data at nodes.");
  else
    ThermochimicaUtils::addClassDescription(
        params, "Provides access to Thermochimica-calculated data at elements.");

  params.addRequiredCoupledVar("elements", "Amounts of elements");
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addCoupledVar("pressure", 1.0, "Pressure");

  MooseEnum reinit_type("none time nodal", "nodal");
  params.addParam<MooseEnum>(
      "reinit_type", reinit_type, "Reinitialization scheme to use with Thermochimica");

  params.addCoupledVar("output_element_potentials", "Chemical potentials of elements");
  params.addCoupledVar("output_phases", "Amounts of phases to be output");
  params.addCoupledVar("output_species", "Amounts of species to be output");
  params.addCoupledVar("output_vapor_pressures", "Vapour pressures of species to be output");
  params.addCoupledVar("output_element_phases",
                       "Elements whose molar amounts in specific phases are requested");

  MooseEnum mUnit_op("mole_fraction moles", "moles");
  params.addParam<MooseEnum>(
      "output_species_unit", mUnit_op, "Mass unit for output species: mole_fractions or moles");

  if constexpr (is_nodal)
    params.set<bool>("unique_node_execute") = true;

  params.addPrivateParam<ChemicalCompositionAction *>("_chemical_composition_action");
  params.addParam<FileName>("thermofile",
                            "Thermodynamic file to be used for Thermochimica calculations");
  return params;
}

void
ThermochimicaDataBase_handler(int /*signum*/)
{
  exit(0);
}

template <bool is_nodal>
ThermochimicaDataBase<is_nodal>::ThermochimicaDataBase(const InputParameters & parameters)
  : ThermochimicaDataBaseParent<is_nodal>(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
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
    _reinit(parameters.get<MooseEnum>("reinit_type").getEnum<ReinitializationType>()),
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
    _el_ph(_n_phase_elements),
    _output_mass_unit(parameters.get<MooseEnum>("output_species_unit").getEnum<OutputMassUnit>())
{
  ThermochimicaUtils::checkLibraryAvailability(*this);

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
  // buffer size
  const auto dofid_size = std::max(/* send */ 1, /* receive */ 0);
  const auto real_size =
      std::max(/* send */ 2 + _n_elements,
               /* receive */ _n_phases + _n_species + _element_potentials.size() +
                   _n_vapor_species + _n_phase_elements);

  // set up shared memory for communication with child process
  auto shared_mem =
      static_cast<std::byte *>(mmap(nullptr,
                                    dofid_size * sizeof(dof_id_type) + real_size * sizeof(Real),
                                    PROT_READ | PROT_WRITE,
                                    MAP_ANONYMOUS | MAP_SHARED,
                                    -1 /* fd */,
                                    0 /* offset */));
  if (shared_mem == MAP_FAILED)
    mooseError("Failed to allocate shared memory for thermochimica IPC.");

  // set up buffer partitions
  _shared_dofid_mem = reinterpret_cast<dof_id_type *>(shared_mem);
  _shared_real_mem = reinterpret_cast<Real *>(shared_mem + dofid_size * sizeof(dof_id_type));

  // set up a bidirectional communication socket
  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
    mooseError("Failed to create socketpair for thermochimica IPC.");

  // fork child process that will manage thermochimica calls
  _pid = fork();
  if (_pid < 0)
    mooseError("Fork failed for thermochimica library.");
  if (_pid == 0)
  {
    // here we are in the child process
    _socket = sockets[0];
    // clean exit upon SIGTERM (mainly for Civet code coverage)
    signal(SIGTERM, ThermochimicaDataBase_handler);

#ifdef THERMOCHIMICA_ENABLED
    // Initialize database in Thermochimica
    if (isParamValid("thermofile"))
    {
      const auto thermo_file = this->template getParam<FileName>("thermofile");

      if (thermo_file.length() > 1024)
        this->paramError("thermofile",
                         "Path exceeds Thermochimica's maximal permissible length of 1024 with ",
                         thermo_file.length(),
                         " characters: ",
                         thermo_file);

      Thermochimica::setThermoFilename(thermo_file);

      // Read in thermodynamics model, only once
      Thermochimica::parseThermoFile();

      const auto idbg = Thermochimica::checkInfoThermo();
      if (idbg != 0)
        this->paramError("thermofile", "Thermochimica data file cannot be parsed. ", idbg);
    }
#endif

    while (true)
      server();
  }

  // parent process continues here
  _socket = sockets[1];
}

template <bool is_nodal>
ThermochimicaDataBase<is_nodal>::~ThermochimicaDataBase()
{
  if (_pid)
    kill(_pid, SIGTERM);
}

template <bool is_nodal>
void
ThermochimicaDataBase<is_nodal>::initialize()
{
}

template <bool is_nodal>
template <typename T>
void
ThermochimicaDataBase<is_nodal>::expect(T expect_msg)
{
  T msg;
  while (read(_socket, &msg, sizeof(T)) == 0)
  {
    if (errno == EAGAIN)
      continue;
    mooseError("Read error waiting for '", expect_msg, "' ", errno, ' ', strerror(errno));
  }
  if (msg != expect_msg)
    mooseError("Expected '", expect_msg, "' but received '", msg, "'");
}

template <bool is_nodal>
template <typename T>
void
ThermochimicaDataBase<is_nodal>::notify(T send_msg)
{
  if (write(_socket, &send_msg, sizeof(T)) != sizeof(T))
    mooseError("Failed to notify thermochimica library child process.");
}

template <bool is_nodal>
void
ThermochimicaDataBase<is_nodal>::execute()
{
  // either one DOF at a node or (currently) one DOF for constant monomial FV!
  // This is enforced automatically by the ChemicalComposition action, which creates the correct
  // variables.
  const unsigned int qp = 0;

  // store current dofID
  if constexpr (is_nodal)
    _shared_dofid_mem[0] = this->_current_node->id();
  else
    _shared_dofid_mem[0] = this->_current_elem->id();

  // store all required data in shared memory
  _shared_real_mem[0] = _temperature[qp];
  _shared_real_mem[1] = _pressure[qp];
  for (const auto i : make_range(_n_elements))
    _shared_real_mem[2 + i] = (*_el[i])[qp];

  // message child process to trigger calculation
  notify('A');

  // and wait for the child process to signal end of calculation
  expect('B');

  // unpack data from shared memory
  std::size_t idx = 0;

  for (const auto i : make_range(_n_phases))
    _ph[i]->setDofValue(_shared_real_mem[idx++], qp);

  for (const auto i : make_range(_n_species))
    _sp[i]->setDofValue(_shared_real_mem[idx++], qp);

  if (_output_element_potentials)
    for (const auto i : index_range(_element_potentials))
      _el_pot[i]->setDofValue(_shared_real_mem[idx++], qp);

  if (_output_vapor_pressures)
    for (const auto i : make_range(_n_vapor_species))
      _vp[i]->setDofValue(_shared_real_mem[idx++], qp);

  if (_output_element_phases)
    for (const auto i : make_range(_n_phase_elements))
      _el_ph[i]->setDofValue(_shared_real_mem[idx++], qp);
}

template <bool is_nodal>
void
ThermochimicaDataBase<is_nodal>::server()
{
  // wait for message from parent process
  expect('A');

#ifdef THERMOCHIMICA_ENABLED
  // fetch data from shared memory
  _current_id = _shared_dofid_mem[0];

  auto temperature = _shared_real_mem[0];
  auto pressure = _shared_real_mem[1];

  // Set temperature and pressure for thermochemistry solver
  Thermochimica::setTemperaturePressure(temperature, pressure);

  // Reset all element masses to 0
  Thermochimica::setElementMass(0, 0.0);

  // Set element masses
  for (const auto i : make_range(_n_elements))
    Thermochimica::setElementMass(_el_ids[i], _shared_real_mem[2 + i]);

  // Optionally ask for a re-initialization (if reinit_requested == true)
  reinitDataMooseToTc();

  // Calculate thermochemical equilibrium
  Thermochimica::thermochimica();

  // Check for error status
  auto idbg = Thermochimica::checkInfoThermo();
  if (idbg != 0)
    // error out for now, but we should send a code to the parent process
    mooseError("Thermochimica error ", idbg);

  // Save data for future reinits
  reinitDataMooseFromTc();

  // Get requested phase indices if phase concentration output was requested
  // i.e. if output_phases is coupled
  auto moles_phase = Thermochimica::getMolesPhase();

  std::size_t idx = 0;

  for (const auto i : make_range(_n_phases))
  {
    // Is this maybe constant? No it isn't for now
    auto [index, idbg] = Thermochimica::getPhaseIndex(_ph_names[i]);
    if (idbg != 0)
      mooseError("Failed to get index of phase '", _ph_names[i], "'");
    // Convert from 1-based (fortran) to 0-based (c++) indexing
    if (index - 1 < 0)
      _shared_real_mem[idx] = 0.0;
    else
      _shared_real_mem[idx] = moles_phase[index - 1];
    idx++;
  }

  auto db_phases = Thermochimica::getPhaseNamesSystem();
  auto getSpeciesMoles =
      [this, moles_phase, db_phases](const std::string phase,
                                     const std::string species) -> std::pair<double, int>
  {
    Real value = 0.0;
    int code = 0;

    auto [index, idbg] = Thermochimica::getPhaseIndex(phase);

    if (Thermochimica::isPhaseMQM(
            std::distance(db_phases.begin(), std::find(db_phases.begin(), db_phases.end(), phase))))
    {
      auto [fraction, idbg] = Thermochimica::getMqmqaPairMolFraction(phase, species);

      switch (_output_mass_unit)
      {
        case OutputMassUnit::FRACTION:
        {
          value = fraction;
          code = idbg;
          break;
        }
        case OutputMassUnit::MOLES:
        {
          auto [molesPair, idbgPair] = Thermochimica::getMqmqaMolesPairs(phase);
          value = molesPair * fraction;
          code = idbg + idbgPair;
          break;
        }
        default:
          break;
      }
    }
    else
    {
      auto [fraction, idbg] = Thermochimica::getOutputMolSpeciesPhase(phase, species);
      switch (_output_mass_unit)
      {
        case OutputMassUnit::FRACTION:
        {
          value = fraction;
          code = idbg;
          break;
        }
        case OutputMassUnit::MOLES:
        {
          value = index >= 1 ? moles_phase[index - 1] * fraction : 0.0;
          code = idbg;
          break;
        }
        default:
          break;
      }
    }
    return {value, code};
  };

  for (const auto i : make_range(_n_species))
  {
    auto [fraction, idbg] = getSpeciesMoles(
        _species_phase_pairs[i].first,
        _species_phase_pairs[i].second); // can we somehow use IDs instead of strings here?

    if (idbg == 0)
      _shared_real_mem[idx] = fraction;
    else if (idbg == 1)
      _shared_real_mem[idx] = 0.0;
#ifndef NDEBUG
    else
      mooseError("Failed to get phase speciation for phase '",
                 _species_phase_pairs[i].first,
                 "' and species '",
                 _species_phase_pairs[i].second,
                 "'. Thermochimica returned ",
                 idbg);
#endif
    idx++;
  }

  if (_output_element_potentials)
    for (const auto i : index_range(_element_potentials))
    {
      auto [potential, idbg] = Thermochimica::getOutputChemPot(_element_potentials[i]);
      if (idbg == 0)
        _shared_real_mem[idx] = potential;
      else if (idbg == 1)
        // element not present, just leave this at 0 for now
        _shared_real_mem[idx] = 0.0;
#ifndef NDEBUG
      else if (idbg == -1)
        mooseError("Failed to get element potential for element '",
                   _element_potentials[i],
                   "'. Thermochimica returned ",
                   idbg);
#endif
      idx++;
    }

  if (_output_vapor_pressures)
    for (const auto i : make_range(_n_vapor_species))
    {
      auto [fraction, moles, idbg] =
          Thermochimica::getOutputMolSpecies(_vapor_phase_pairs[i].second);
      libmesh_ignore(moles);

      if (idbg == 0)
        _shared_real_mem[idx] = fraction * pressure;
      else if (idbg == 1)
        _shared_real_mem[idx] = 0.0;
#ifndef NDEBUG
      else
        mooseError("Failed to get vapor pressure for phase '",
                   _vapor_phase_pairs[i].first,
                   "' and species '",
                   _vapor_phase_pairs[i].second,
                   "'. Thermochimica returned ",
                   idbg);
#endif
      idx++;
    }

  if (_output_element_phases)
    for (const auto i : make_range(_n_phase_elements))
    {
      auto [moles, idbg] = Thermochimica::getElementMolesInPhase(_phase_element_pairs[i].second,
                                                                 _phase_element_pairs[i].first);

      if (idbg == 0)
        _shared_real_mem[idx] = moles;
      else if (idbg == 1)
        _shared_real_mem[idx] = 0.0;
#ifndef NDEBUG
      else
        mooseError("Failed to get moles of element '",
                   _phase_element_pairs[i].second,
                   "' in phase '",
                   _phase_element_pairs[i].first,
                   "'. Thermochimica returned ",
                   idbg);
#endif
      idx++;
    }
#endif
  // Send message back to parent process
  notify('B');
}

template <bool is_nodal>
void
ThermochimicaDataBase<is_nodal>::reinitDataMooseFromTc()
{
#ifdef THERMOCHIMICA_ENABLED
  auto & d = _data[_current_id];

  if (_reinit != ReinitializationType::NONE)
  {
    Thermochimica::saveReinitData();
    auto data = Thermochimica::getReinitData();

    if (_reinit == ReinitializationType::TIME)
    {
      d._assemblage = std::move(data.assemblage);
      d._moles_phase = std::move(data.molesPhase);
      d._element_potential = std::move(data.elementPotential);
      d._chemical_potential = std::move(data.chemicalPotential);
      d._mol_fraction = std::move(data.moleFraction);
      d._elements_used = std::move(data.elementsUsed);
      d._reinit_available = data.reinitAvailable;
    }
  }
#endif
}

template <bool is_nodal>
void
ThermochimicaDataBase<is_nodal>::reinitDataMooseToTc()
{
#ifdef THERMOCHIMICA_ENABLED
  // Tell Thermochimica whether a re-initialization is requested for this calculation
  switch (_reinit)
  {
    case ReinitializationType::NONE:
      Thermochimica::setReinitRequested(false);
      break;
    default:
      Thermochimica::setReinitRequested(true);
  }
  // If we have re-initialization data and want a re-initialization, then
  // load data into Thermochimica
  auto it = _data.find(_current_id);

  if (it != _data.end() &&
      _reinit == ReinitializationType::TIME) // If doing previous timestep reinit
  {
    auto & d = it->second;
    if (d._reinit_available)
    {
      Thermochimica::resetReinit();
      Thermochimica::ReinitializationData data;
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

template <bool is_nodal>
const typename ThermochimicaDataBase<is_nodal>::Data &
ThermochimicaDataBase<is_nodal>::getNodalData(dof_id_type node_id) const
{
  if constexpr (!is_nodal)
    mooseError("Requesting nodal data from an element object.");
  return this->getData(node_id);
}

template <bool is_nodal>
const typename ThermochimicaDataBase<is_nodal>::Data &
ThermochimicaDataBase<is_nodal>::getElementData(dof_id_type element_id) const
{
  if constexpr (is_nodal)
    mooseError("Requesting per element data from a nodal object.");
  return this->getData(element_id);
}

template <bool is_nodal>
const typename ThermochimicaDataBase<is_nodal>::Data &
ThermochimicaDataBase<is_nodal>::getData(dof_id_type id) const
{
  const auto it = _data.find(id);
  if (it == _data.end())
  {
    if constexpr (is_nodal)
      mooseError("Unable to look up data for node ", id);
    else
      mooseError("Unable to look up data for element ", id);
  }
  return it->second;
}

template class ThermochimicaDataBase<true>;
template class ThermochimicaDataBase<false>;
