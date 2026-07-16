//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermochimicaData.h"
#include "ThermochimicaUtils.h"
#include "MooseVariable.h"
#include "MooseVariableField.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "FEProblemBase.h"
#include "AuxiliarySystem.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/threads.h"

#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <new>
#include <numeric>

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#include "checkUnits.h"
#endif

registerMooseObject("ChemicalReactionsApp", ThermochimicaData);

namespace
{
Threads::spin_mutex output_mutex;

std::size_t
alignedOffset(const std::size_t offset, const std::size_t alignment)
{
  return (offset + alignment - 1) & ~(alignment - 1);
}

#ifdef THERMOCHIMICA_ENABLED
int
currentPhaseIndex(const std::string & phase)
{
  const auto phases = Thermochimica::getPhaseNamesSystem();
  const auto found = std::find(phases.begin(), phases.end(), phase);
  return found == phases.end() ? -1 : static_cast<int>(std::distance(phases.begin(), found));
}

int
currentComponentIndex(const int phase_index,
                      const std::string & component,
                      const ThermochimicaConfiguration::ChemicalPotentialKind kind)
{
  const auto components = kind == ThermochimicaConfiguration::ChemicalPotentialKind::ENDMEMBER
                              ? Thermochimica::getSpeciesInPhase(phase_index)
                              : Thermochimica::getThermodynamicSpeciesInPhase(phase_index);
  const auto found = std::find(components.begin(), components.end(), component);
  return found == components.end() ? -1
                                   : static_cast<int>(std::distance(components.begin(), found));
}
#endif
}

InputParameters
ThermochimicaData::validParams()
{
  InputParameters params = ThreadedGeneralUserObject::validParams();
  params += BlockRestrictable::validParams();
  ThermochimicaUtils::addClassDescription(
      params, "Internal exact batched Thermochimica equilibrium executor.");
  params.addPrivateParam<ThermochimicaConfigurationPtr>("_configuration");
  params.addCoupledVar("temperature", "Temperature input used to establish execution dependencies");
  params.addCoupledVar("pressure", 1.0, "Pressure input used to establish execution dependencies");
  params.addCoupledVar("elements", "Element inputs used to establish execution dependencies");
  return params;
}

ThermochimicaData::ThermochimicaData(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    BlockRestrictable(this),
    _configuration(parameters.get<ThermochimicaConfigurationPtr>("_configuration")),
    _nodal(_configuration->location == ThermochimicaConfiguration::EvaluationLocation::NODAL),
    _thread_count(libMesh::n_threads()),
    _temperature(inputSource(_configuration->temperature)),
    _pressure(inputSource(_configuration->pressure))
{
  ThermochimicaUtils::checkLibraryAvailability(*this);

  _elements.reserve(_configuration->element_variables.size());
  for (const auto & name : _configuration->element_variables)
    _elements.push_back(
        &dynamic_cast<MooseVariableField<Real> &>(_subproblem.getActualFieldVariable(_tid, name)));

  _outputs.reserve(_configuration->outputs.size());
  for (const auto & output : _configuration->outputs)
    std::visit(
        [&](const auto & descriptor)
        {
          _outputs.push_back(&dynamic_cast<MooseVariableField<Real> &>(
              _subproblem.getActualFieldVariable(_tid, descriptor.variable)));
        },
        output);

  createWorker();
}

ThermochimicaData::~ThermochimicaData() { destroyWorker(); }

void
ThermochimicaData::initialize()
{
  _evaluated_states = 0;
  _batches = 0;
  _warm_starts = 0;
  _solve_seconds = 0;
  _packing_seconds = 0;
  _ipc_seconds = 0;
}

ThermochimicaData::InputSource
ThermochimicaData::inputSource(const std::string & value)
{
  InputSource source;
  if (_subproblem.hasVariable(value))
    source.variable =
        &dynamic_cast<MooseVariableField<Real> &>(_subproblem.getActualFieldVariable(_tid, value));
  else
  {
    try
    {
      source.constant = MooseUtils::convert<Real>(value);
    }
    catch (...)
    {
      mooseError("Thermochimica input '", value, "' is neither a field variable nor a number.");
    }
  }
  return source;
}

Real
ThermochimicaData::inputValue(const InputSource & source, const bool nodal) const
{
  if (!source.variable)
    return source.constant;
  if (nodal)
  {
    const auto * variable = dynamic_cast<const MooseVariable *>(source.variable);
    if (!variable)
      mooseError("Nodal Thermochimica inputs must be nodal finite-element variables.");
    return variable->nodalValue();
  }
  return source.variable->sln()[0];
}

bool
ThermochimicaData::ownsEntity(const dof_id_type id) const
{
  return id % _thread_count == _tid;
}

bool
ThermochimicaData::includesElement(const libMesh::Elem & elem) const
{
  return hasBlocks(elem.subdomain_id());
}

bool
ThermochimicaData::includesNode(const libMesh::Node & node) const
{
  const auto & node_blocks = _fe_problem.mesh().getNodeBlockIds(node);
  return std::any_of(
      node_blocks.begin(), node_blocks.end(), [this](const auto id) { return hasBlocks(id); });
}

void
ThermochimicaData::execute()
{
  unsigned int row = 0;
  auto store_inputs = [&](const auto & entity)
  {
    const auto packing_start = std::chrono::steady_clock::now();
    const auto id = entity.id();
    _entity_ids[row] = id;
    auto * input = _inputs + row * _configuration->inputWidth();
    input[0] = inputValue(_temperature, _nodal);
    input[1] = inputValue(_pressure, _nodal);
    for (const auto i : index_range(_elements))
      input[2 + i] = inputValue({_elements[i], 0}, _nodal);
    _packing_seconds +=
        std::chrono::duration<Real>(std::chrono::steady_clock::now() - packing_start).count();

    ++row;
    if (row == _configuration->batch_size)
    {
      flushBatch(row);
      row = 0;
    }
  };

  if (_nodal)
  {
    auto & mesh = _fe_problem.mesh().getMesh();
    for (auto it = mesh.local_nodes_begin(); it != mesh.local_nodes_end(); ++it)
    {
      const auto & node = **it;
      if (!ownsEntity(node.id()) || !includesNode(node))
        continue;
      _fe_problem.reinitNode(&node, _tid);
      store_inputs(node);
    }
  }
  else
  {
    auto & mesh = _fe_problem.mesh().getMesh();
    for (auto it = mesh.active_local_elements_begin(); it != mesh.active_local_elements_end(); ++it)
    {
      const auto & elem = **it;
      if (!ownsEntity(elem.id()) || !includesElement(elem))
        continue;
      _fe_problem.reinitElem(&elem, _tid);
      store_inputs(elem);
    }
  }

  if (row)
    flushBatch(row);
}

void
ThermochimicaData::threadJoin(const UserObject & other)
{
  const auto & data = static_cast<const ThermochimicaData &>(other);
  _evaluated_states += data._evaluated_states;
  _batches += data._batches;
  _warm_starts += data._warm_starts;
  _solve_seconds += data._solve_seconds;
  _packing_seconds += data._packing_seconds;
  _ipc_seconds += data._ipc_seconds;
}

void
ThermochimicaData::finalize()
{
  if (!_outputs.empty())
  {
    auto & auxiliary = _fe_problem.getAuxiliarySystem();
    auxiliary.solution().close();
    auxiliary.update();
  }
  if (_configuration->report_performance)
    _console << "ThermochimicaData '" << name() << "': states=" << _evaluated_states
             << ", batches=" << _batches << ", exact_solves=" << _evaluated_states
             << ", warm_starts=" << _warm_starts << ", worker_solve_time=" << _solve_seconds
             << " s, packing_time=" << _packing_seconds << " s, ipc_time=" << _ipc_seconds << " s"
             << std::endl;
}

void
ThermochimicaData::createWorker()
{
  std::size_t offset = sizeof(SharedHeader);
  offset = alignedOffset(offset, alignof(dof_id_type));
  const auto ids_offset = offset;
  offset += _configuration->batch_size * sizeof(dof_id_type);
  offset = alignedOffset(offset, alignof(int));
  const auto status_offset = offset;
  offset += _configuration->batch_size * sizeof(int);
  offset = alignedOffset(offset, alignof(Real));
  const auto inputs_offset = offset;
  offset += _configuration->batch_size * _configuration->inputWidth() * sizeof(Real);
  const auto results_offset = offset;
  offset += _configuration->batch_size * _configuration->outputWidth() * sizeof(Real);
  _shared_memory_size = offset;

  _shared_memory =
      mmap(nullptr, _shared_memory_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  if (_shared_memory == MAP_FAILED)
    mooseError("Failed to allocate shared memory for the Thermochimica worker: ", strerror(errno));

  auto * bytes = static_cast<std::byte *>(_shared_memory);
  _header = reinterpret_cast<SharedHeader *>(bytes);
  new (_header) SharedHeader();
  _entity_ids = reinterpret_cast<dof_id_type *>(bytes + ids_offset);
  _row_status = reinterpret_cast<int *>(bytes + status_offset);
  _inputs = reinterpret_cast<Real *>(bytes + inputs_offset);
  _results = reinterpret_cast<Real *>(bytes + results_offset);

  int sockets[2];
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) != 0)
  {
    const auto socket_errno = errno;
    munmap(_shared_memory, _shared_memory_size);
    _shared_memory = nullptr;
    errno = socket_errno;
    mooseError("Failed to create a socket for the Thermochimica worker: ", strerror(errno));
  }

  _worker_pid = fork();
  if (_worker_pid < 0)
  {
    const auto fork_errno = errno;
    close(sockets[0]);
    close(sockets[1]);
    munmap(_shared_memory, _shared_memory_size);
    _shared_memory = nullptr;
    errno = fork_errno;
    mooseError("Failed to fork the Thermochimica worker: ", strerror(errno));
  }
  if (_worker_pid == 0)
  {
    close(sockets[1]);
    _socket = sockets[0];
    workerLoop();
  }

  close(sockets[0]);
  _socket = sockets[1];
  try
  {
    if (readMessage() != 'I')
      mooseError("Thermochimica worker failed during initialization.");
    if (_header->worker_status)
      mooseError(
          "Thermochimica worker initialization failed with status ", _header->worker_status, ".");
  }
  catch (...)
  {
    destroyWorker();
    throw;
  }
}

void
ThermochimicaData::destroyWorker()
{
  if (_worker_pid > 0)
  {
    _header->command = static_cast<unsigned int>(Command::STOP);
    try
    {
      writeMessage('Q');
      readMessage();
    }
    catch (...)
    {
    }
    if (_socket >= 0)
    {
      close(_socket);
      _socket = -1;
    }
    int status = 0;
    while (waitpid(_worker_pid, &status, 0) < 0 && errno == EINTR)
      ;
    _worker_pid = -1;
  }
  if (_socket >= 0)
  {
    close(_socket);
    _socket = -1;
  }
  if (_shared_memory && _shared_memory != MAP_FAILED)
  {
    munmap(_shared_memory, _shared_memory_size);
    _shared_memory = nullptr;
  }
}

void
ThermochimicaData::writeMessage(const char message)
{
  ssize_t written;
  do
    written = send(_socket, &message, sizeof(message), MSG_NOSIGNAL);
  while (written < 0 && errno == EINTR);
  if (written != sizeof(message))
    mooseError("Thermochimica worker communication failed while writing: ", strerror(errno));
}

char
ThermochimicaData::readMessage()
{
  char message = 0;
  ssize_t received;
  do
    received = recv(_socket, &message, sizeof(message), 0);
  while (received < 0 && errno == EINTR);
  if (received == 0)
    mooseError("Thermochimica worker exited unexpectedly.");
  if (received != sizeof(message))
    mooseError("Thermochimica worker communication failed while reading: ", strerror(errno));
  return message;
}

void
ThermochimicaData::flushBatch(const unsigned int count)
{
  _header->command = static_cast<unsigned int>(Command::SOLVE);
  _header->count = count;
  _header->worker_status = 0;
  _header->warm_starts = 0;
  _header->solve_seconds = 0;
  const auto ipc_start = std::chrono::steady_clock::now();
  writeMessage('Q');
  if (readMessage() != 'R')
    mooseError("Thermochimica worker returned an invalid response.");
  const auto round_trip_seconds =
      std::chrono::duration<Real>(std::chrono::steady_clock::now() - ipc_start).count();
  if (_header->worker_status)
    mooseError("Thermochimica worker failed with status ", _header->worker_status, ".");

  for (const auto row : make_range(count))
  {
    if (_row_status[row])
      mooseError("Thermochimica failed for entity ",
                 _entity_ids[row],
                 " with status ",
                 _row_status[row],
                 ".");
    publishRow(row);
  }

  _evaluated_states += count;
  ++_batches;
  _warm_starts += _header->warm_starts;
  _solve_seconds += _header->solve_seconds;
  _ipc_seconds += std::max<Real>(0.0, round_trip_seconds - _header->solve_seconds);
}

void
ThermochimicaData::publishRow(const unsigned int row)
{
  if (_outputs.empty())
    return;

  const libMesh::DofObject * entity =
      _nodal ? static_cast<const libMesh::DofObject *>(
                   _fe_problem.mesh().getMesh().node_ptr(_entity_ids[row]))
             : static_cast<const libMesh::DofObject *>(
                   _fe_problem.mesh().getMesh().elem_ptr(_entity_ids[row]));
  if (!entity)
    mooseError("Unable to find Thermochimica output entity ", _entity_ids[row], ".");

  Threads::spin_mutex::scoped_lock lock(output_mutex);
  auto & solution = _fe_problem.getAuxiliarySystem().solution();
  const auto * result = _results + row * _configuration->outputWidth();
  for (const auto output : index_range(_outputs))
  {
    const auto dof =
        entity->dof_number(_outputs[output]->sys().number(), _outputs[output]->number(), 0);
    solution.set(dof, result[output]);
  }
}

void
ThermochimicaData::initializeThermochimica()
{
#ifdef THERMOCHIMICA_ENABLED
  Thermochimica::setThermoFilename(_configuration->database);
  Thermochimica::parseThermoFile();
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
  {
    _header->worker_status = info;
    return;
  }
  Thermochimica::checkTemperature(_configuration->temperature_unit);
  Thermochimica::setUnitTemperature(_configuration->temperature_unit);
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
  {
    _header->worker_status = info;
    return;
  }
  Thermochimica::checkPressure(_configuration->pressure_unit);
  Thermochimica::setUnitPressure(_configuration->pressure_unit);
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
  {
    _header->worker_status = info;
    return;
  }
  Thermochimica::checkMass(_configuration->composition_unit);
  Thermochimica::setUnitMass(_configuration->composition_unit);
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
  {
    _header->worker_status = info;
    return;
  }

  int selection_info = 0;
  if (_configuration->phase_selection == ThermochimicaConfiguration::PhaseSelection::INCLUDE)
    selection_info = Thermochimica::setIncludedPhases(_configuration->selected_phases);
  else
    selection_info = Thermochimica::setExcludedPhases(_configuration->selected_phases);
  if (selection_info != 0)
  {
    _header->worker_status = selection_info;
    return;
  }
  Thermochimica::setHeatCapacityEnthalpyEntropyRequested(
      _configuration->needs_system_properties);
#endif
}

[[noreturn]] void
ThermochimicaData::workerLoop()
{
  initializeThermochimica();
  writeMessage('I');
  if (_header->worker_status)
    _exit(1);
  while (true)
  {
    readMessage();
    if (_header->command == static_cast<unsigned int>(Command::STOP))
    {
      writeMessage('R');
      _exit(0);
    }
    if (_header->command != static_cast<unsigned int>(Command::SOLVE))
    {
      _header->worker_status = EINVAL;
      writeMessage('R');
      continue;
    }

    const auto start = std::chrono::steady_clock::now();
    for (const auto row : make_range(_header->count))
      _row_status[row] = solveRow(row);
    _header->solve_seconds =
        std::chrono::duration<Real>(std::chrono::steady_clock::now() - start).count();
    writeMessage('R');
  }
}

int
ThermochimicaData::solveRow(const unsigned int row)
{
#ifdef THERMOCHIMICA_ENABLED
  const auto * input = _inputs + row * _configuration->inputWidth();
  auto * result = _results + row * _configuration->outputWidth();
  std::fill(result, result + _configuration->outputWidth(), 0.0);
  _current_entity = _entity_ids[row];

  Thermochimica::setTemperaturePressure(input[0], input[1]);
  Thermochimica::setElementMass(0, 0.0);
  for (const auto i : index_range(_configuration->element_ids))
    Thermochimica::setElementMass(_configuration->element_ids[i], input[2 + i]);

  const auto warm_start = _configuration->warm_start;
  if (warm_start == ThermochimicaConfiguration::WarmStart::PREVIOUS_TIMESTEP)
  {
    Thermochimica::resetReinit();
    if (loadPreviousState(_current_entity))
    {
      Thermochimica::setReinitRequested(true);
      ++_header->warm_starts;
    }
    else
      Thermochimica::setReinitRequested(false);
  }
  else
  {
    Thermochimica::setReinitRequested(warm_start ==
                                      ThermochimicaConfiguration::WarmStart::PREVIOUS_SOLVE);
    if (warm_start == ThermochimicaConfiguration::WarmStart::PREVIOUS_SOLVE &&
        _worker_has_previous_solve)
      ++_header->warm_starts;
  }

  Thermochimica::thermochimica();
  if (const auto info = Thermochimica::checkInfoThermo(); info != 0)
    return info;

  if (warm_start != ThermochimicaConfiguration::WarmStart::NONE)
  {
    Thermochimica::saveReinitData();
    if (warm_start == ThermochimicaConfiguration::WarmStart::PREVIOUS_TIMESTEP)
      storePreviousState(_current_entity);
    else if (warm_start == ThermochimicaConfiguration::WarmStart::PREVIOUS_SOLVE)
      _worker_has_previous_solve = true;
  }

  const auto total_input =
      std::accumulate(input + 2, input + _configuration->inputWidth(), Real(0));
  const bool use_indexed_outputs = std::all_of(input + 2,
                                               input + _configuration->inputWidth(),
                                               [total_input](const Real value)
                                               { return value > std::abs(total_input) * 1e-10; });
  const auto moles_phase =
      use_indexed_outputs ? std::vector<double>() : Thermochimica::getMolesPhase();
  Real phase_total = 0.0;
  if (_configuration->needs_phase_total)
  {
    for (const auto phase : index_range(_configuration->phase_indices))
      if (use_indexed_outputs)
      {
        const auto result = Thermochimica::getPhaseMoles(_configuration->phase_indices[phase]);
        if (result.second != 0)
          return result.second;
        if (result.first > 0.0)
          phase_total += result.first;
      }
      else
      {
        const auto [phase_index, phase_info] =
            Thermochimica::getPhaseIndex(_configuration->phase_names[phase]);
        if (phase_info != 0)
          return phase_info;
        if (phase_index > 0 && moles_phase[phase_index - 1] > 0.0)
          phase_total += moles_phase[phase_index - 1];
      }
  }
  OutputEvaluationContext context{use_indexed_outputs, moles_phase, phase_total, input[1], {}};
  for (const auto output : index_range(_configuration->outputs))
  {
    const auto info = std::visit([&](const auto & descriptor)
                                 { return evaluateOutput(descriptor, context, result[output]); },
                                 _configuration->outputs[output]);
    if (info != 0)
      return info;
  }
  return 0;
#else
  return ENOSYS;
#endif
}

#ifdef THERMOCHIMICA_ENABLED
int
ThermochimicaData::evaluateOutput(const ThermochimicaConfiguration::PhaseOutput & output,
                                  OutputEvaluationContext & context,
                                  Real & value) const
{
  int info = 0;
  if (context.use_indexed_outputs)
  {
    const auto [moles, phase_info] = Thermochimica::getPhaseMoles(output.phase_index);
    value = moles;
    info = phase_info;
  }
  else
  {
    const auto [index, phase_info] = Thermochimica::getPhaseIndex(output.phase);
    value = phase_info == 0 && index > 0 ? context.moles_phase[index - 1] : 0.0;
    info = phase_info;
  }
  if (info == 0 && output.unit == ThermochimicaConfiguration::AmountUnit::MOLE_FRACTION)
    value = value > 0.0 && context.phase_total > 0.0 ? value / context.phase_total : 0.0;
  return info;
}

int
ThermochimicaData::evaluateOutput(const ThermochimicaConfiguration::SpeciesOutput & output,
                                  OutputEvaluationContext & context,
                                  Real & value) const
{
  int info = 0;
  if (output.is_mqm)
  {
    const auto pair = Thermochimica::getMqmqaPairMolFraction(output.phase, output.species);
    value = std::get<0>(pair);
    info = std::get<1>(pair);
    if (output.unit == ThermochimicaConfiguration::AmountUnit::MOLES && info == 0)
    {
      const auto pairs = Thermochimica::getMqmqaMolesPairs(output.phase);
      value *= std::get<0>(pairs);
      info += std::get<1>(pairs);
    }
  }
  else
  {
    const auto species_value =
        context.use_indexed_outputs
            ? Thermochimica::getOutputMolSpeciesPhase(output.phase_index, output.species_index)
            : Thermochimica::getOutputMolSpeciesPhase(output.phase, output.species);
    value = std::get<0>(species_value);
    info = std::get<1>(species_value);
    if (output.unit == ThermochimicaConfiguration::AmountUnit::MOLES)
    {
      if (context.use_indexed_outputs)
      {
        const auto phase = Thermochimica::getPhaseMoles(output.phase_index);
        value *= phase.first;
        if (phase.second != 0)
          return phase.second;
      }
      else
      {
        const auto [phase_index, phase_info] = Thermochimica::getPhaseIndex(output.phase);
        if (phase_info != 0)
          return phase_info;
        value = phase_index > 0 ? value * context.moles_phase[phase_index - 1] : 0.0;
      }
    }
  }

  if (info != 0 && info != 1)
    return info;
  if (info == 1)
    value = 0.0;
  return 0;
}

int
ThermochimicaData::evaluateOutput(const ThermochimicaConfiguration::ElementPotentialOutput & output,
                                  OutputEvaluationContext & context,
                                  Real & value) const
{
  const auto result = context.use_indexed_outputs
                          ? Thermochimica::getElementPotential(output.element_index)
                          : Thermochimica::getOutputChemPot(output.element);
  value = result.first;
  if (result.second != 0 && result.second != 1)
    return result.second;
  if (result.second == 1)
    value = 0.0;
  return 0;
}

int
ThermochimicaData::evaluateOutput(const ThermochimicaConfiguration::VaporPressureOutput & output,
                                  OutputEvaluationContext & context,
                                  Real & value) const
{
  const auto result =
      context.use_indexed_outputs
          ? Thermochimica::getOutputMolSpeciesPhase(output.phase_index, output.species_index)
          : Thermochimica::getOutputMolSpeciesPhase(output.phase, output.species);
  value = result.first * context.pressure;
  if (result.second != 0 && result.second != 1)
    return result.second;
  if (result.second == 1)
    value = 0.0;
  return 0;
}

int
ThermochimicaData::evaluateOutput(
    const ThermochimicaConfiguration::ElementDistributionOutput & output,
    OutputEvaluationContext & context,
    Real & value) const
{
  const auto result =
      context.use_indexed_outputs
          ? Thermochimica::getElementMolesInPhase(output.element_index, output.phase_index)
          : Thermochimica::getElementMolesInPhase(output.element, output.phase);
  value = result.second == 0 ? result.first : 0.0;
  if (result.second != 0 && result.second != 1 && result.second != 2)
    return result.second;

  if (output.unit == ThermochimicaConfiguration::DistributionUnit::FRACTION && value > 0.0)
  {
    const auto [it, inserted] = context.element_totals.try_emplace(output.element_index, 0.0, 0);
    if (inserted)
      for (const auto phase : index_range(_configuration->phase_names))
      {
        const auto phase_result =
            context.use_indexed_outputs
                ? Thermochimica::getElementMolesInPhase(output.element_index,
                                                        _configuration->phase_indices[phase])
                : Thermochimica::getElementMolesInPhase(output.element,
                                                        _configuration->phase_names[phase]);
        if (phase_result.second == 0 && phase_result.first > 0.0)
          it->second.first += phase_result.first;
        else if (phase_result.second != 1 && phase_result.second != 2)
        {
          it->second.second = phase_result.second;
          break;
        }
      }
    if (it->second.second != 0)
      return it->second.second;
    value = it->second.first == 0.0 ? 0.0 : value / it->second.first;
  }
  else if (output.unit == ThermochimicaConfiguration::DistributionUnit::FRACTION)
    value = 0.0;
  return 0;
}

int
ThermochimicaData::evaluateOutput(
    const ThermochimicaConfiguration::ChemicalPotentialOutput & output,
    OutputEvaluationContext & context,
    Real & value) const
{
  const auto phase_index =
      context.use_indexed_outputs ? output.phase_index : currentPhaseIndex(output.phase);
  if (phase_index < 0)
  {
    value = 0.0;
    return 0;
  }
  const auto component_index =
      context.use_indexed_outputs
          ? output.component_index
          : currentComponentIndex(phase_index, output.component, output.kind);
  if (component_index < 0)
  {
    value = 0.0;
    return 0;
  }

  const auto result =
      output.kind == ThermochimicaConfiguration::ChemicalPotentialKind::ENDMEMBER
          ? Thermochimica::getMqmqaEndmemberStoichiometricPotential(phase_index, component_index)
          : Thermochimica::getSpeciesChemicalPotential(phase_index, component_index);
  value = result.second == 0 ? result.first : 0.0;
  return result.second == 1 ? 0 : result.second;
}

int
ThermochimicaData::evaluateOutput(const ThermochimicaConfiguration::PhaseGibbsEnergyOutput & output,
                                  OutputEvaluationContext & context,
                                  Real & value) const
{
  const auto phase_index =
      context.use_indexed_outputs ? output.phase_index : currentPhaseIndex(output.phase);
  if (phase_index < 0)
  {
    value = 0.0;
    return 0;
  }
  const auto result = Thermochimica::getPhaseGibbsEnergy(phase_index);
  value = output.unit == ThermochimicaConfiguration::GibbsEnergyUnit::JOULES ? result.total
                                                                             : result.molar;
  if (result.status == 1)
  {
    value = 0.0;
    return 0;
  }
  return result.status;
}

int
ThermochimicaData::evaluateOutput(
    const ThermochimicaConfiguration::PhaseDrivingForceOutput & output,
    OutputEvaluationContext & context,
    Real & value) const
{
  const auto phase_index =
      context.use_indexed_outputs ? output.phase_index : currentPhaseIndex(output.phase);
  if (phase_index < 0)
  {
    value = 0.0;
    return 0;
  }
  const auto result = Thermochimica::getPhaseDrivingForce(phase_index);
  value = result.second == 0 ? result.first : 0.0;
  return result.second == 1 ? 0 : result.second;
}

int
ThermochimicaData::evaluateOutput(const ThermochimicaConfiguration::SystemGibbsEnergyOutput &,
                                  OutputEvaluationContext &,
                                  Real & value) const
{
  const auto result = Thermochimica::getSystemGibbsEnergy();
  value = result.first;
  return result.second;
}

int
ThermochimicaData::evaluateOutput(const ThermochimicaConfiguration::SystemPropertyOutput & output,
                                  OutputEvaluationContext &,
                                  Real & value) const
{
  const auto properties = Thermochimica::getHeatCapacityEnthalpyEntropy();
  if (output.property == ThermochimicaConfiguration::SystemPropertyKind::ENTHALPY)
    value = std::get<1>(properties);
  else if (output.property == ThermochimicaConfiguration::SystemPropertyKind::ENTROPY)
    value = std::get<2>(properties);
  else
    value = std::get<0>(properties);
  return 0;
}

bool
ThermochimicaData::loadPreviousState(const dof_id_type id)
{
  const auto slot_it = _previous_state_slots.find(id);
  if (slot_it == _previous_state_slots.end() || !_previous_state_available[slot_it->second])
    return false;

  const auto slot = slot_it->second;
  const auto integer_width = _reinit_elements + 169;
  const auto real_width = 2 * _reinit_elements + 2 * _reinit_species;
  auto * integers = _previous_state_integers.data() + slot * integer_width;
  auto * reals = _previous_state_reals.data() + slot * real_width;

  Thermochimica::setReinitData({integers,
                                reals,
                                reals + _reinit_elements,
                                reals + 2 * _reinit_elements,
                                reals + 2 * _reinit_elements + _reinit_species,
                                integers + _reinit_elements});
  return true;
}

void
ThermochimicaData::storePreviousState(const dof_id_type id)
{
  if (!_reinit_elements)
  {
    const auto sizes = Thermochimica::getReinitDataSizes();
    _reinit_elements = sizes.first;
    _reinit_species = sizes.second;
  }

  auto [slot_it, inserted] = _previous_state_slots.emplace(id, _previous_state_slots.size());
  const auto slot = slot_it->second;
  const auto integer_width = _reinit_elements + 169;
  const auto real_width = 2 * _reinit_elements + 2 * _reinit_species;
  if (inserted)
  {
    _previous_state_integers.resize((slot + 1) * integer_width);
    _previous_state_reals.resize((slot + 1) * real_width);
    _previous_state_available.resize(slot + 1);
  }

  auto * integers = _previous_state_integers.data() + slot * integer_width;
  auto * reals = _previous_state_reals.data() + slot * real_width;
  const auto [available, iterations] =
      Thermochimica::getReinitData({integers,
                                    reals,
                                    reals + _reinit_elements,
                                    reals + 2 * _reinit_elements,
                                    reals + 2 * _reinit_elements + _reinit_species,
                                    integers + _reinit_elements});
  (void)iterations;
  _previous_state_available[slot] = available;
}
#endif
