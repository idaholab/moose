//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedGeneralUserObject.h"
#include "BlockRestrictable.h"
#include "ThermochimicaConfiguration.h"
#include "libmesh/dof_object.h"

#include <unordered_map>
#include <sys/types.h>

#ifdef THERMOCHIMICA_ENABLED
#include "Thermochimica-cxx.h"
#endif

template <typename OutputType>
class MooseVariableField;
namespace libMesh
{
class Node;
class Elem;
}

/**
 * Executes exact Thermochimica equilibrium calculations in batches.
 *
 * Each threaded copy owns an isolated worker process because Thermochimica stores its state in
 * Fortran modules. The action is the only supported way to construct this object.
 */
class ThermochimicaData : public ThreadedGeneralUserObject, public BlockRestrictable
{
public:
  static InputParameters validParams();
  ThermochimicaData(const InputParameters & parameters);
  ~ThermochimicaData() override;

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject & other) override;
  void finalize() override;

protected:
  struct InputSource
  {
    MooseVariableField<Real> * variable = nullptr;
    Real constant = 0;
  };

  struct SharedHeader
  {
    unsigned int command = 0;
    unsigned int count = 0;
    unsigned int warm_starts = 0;
    int worker_status = 0;
    Real solve_seconds = 0;
  };

  enum class Command : unsigned int
  {
    SOLVE = 1,
    STOP = 2
  };

  void createWorker();
  void destroyWorker();
  [[noreturn]] void workerLoop();
  void initializeThermochimica();
  int solveRow(unsigned int row);
#ifdef THERMOCHIMICA_ENABLED
  bool loadPreviousState(dof_id_type id);
  void storePreviousState(dof_id_type id);
#endif
  void flushBatch(unsigned int count);
  void publishRow(unsigned int row);

  InputSource inputSource(const std::string & value);
  Real inputValue(const InputSource & source, bool nodal) const;
  bool ownsEntity(dof_id_type id) const;
  bool includesNode(const libMesh::Node & node) const;
  bool includesElement(const libMesh::Elem & elem) const;

  void writeMessage(char message);
  char readMessage();

  const ThermochimicaConfigurationPtr _configuration;
  const bool _nodal;
  const unsigned int _thread_count;

  InputSource _temperature;
  InputSource _pressure;
  std::vector<MooseVariableField<Real> *> _elements;
  std::vector<MooseVariableField<Real> *> _outputs;

  int _socket = -1;
  pid_t _worker_pid = -1;
  void * _shared_memory = nullptr;
  std::size_t _shared_memory_size = 0;
  SharedHeader * _header = nullptr;
  dof_id_type * _entity_ids = nullptr;
  int * _row_status = nullptr;
  Real * _inputs = nullptr;
  Real * _results = nullptr;

  dof_id_type _current_entity = libMesh::DofObject::invalid_id;
  unsigned long _evaluated_states = 0;
  unsigned long _batches = 0;
  unsigned long _warm_starts = 0;
  Real _solve_seconds = 0;
  Real _packing_seconds = 0;
  Real _ipc_seconds = 0;

#ifdef THERMOCHIMICA_ENABLED
  int _reinit_elements = 0;
  int _reinit_species = 0;
  std::unordered_map<dof_id_type, std::size_t> _previous_state_slots;
  std::vector<int> _previous_state_integers;
  std::vector<Real> _previous_state_reals;
  std::vector<unsigned char> _previous_state_available;
#endif
  bool _worker_has_previous_solve = false;
};
