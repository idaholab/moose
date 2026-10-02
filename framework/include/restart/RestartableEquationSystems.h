//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <iostream>

#include "libmesh/enum_order.h"
#include "libmesh/fe_type.h"
#include "libmesh/equation_systems.h"
#include "nlohmann/json.h"

namespace libMesh
{
class DofObject;
class EquationSystems;
class MeshBase;
}

/**
 * Wrapper class that owns a libMesh EquationSystem and adds advanced restart capability to it
 */
class RestartableEquationSystems
{
public:
  RestartableEquationSystems(libMesh::MeshBase & mesh);

  /**
   * Represents a stored variable in restart
   */
  struct VectorHeader
  {
    bool operator==(const VectorHeader & other) const
    {
      return name == other.name && projections == other.projections &&
             variable_offset == other.variable_offset && vector == other.vector;
    }

    /// The name of the stored vector
    std::string name;
    /// The type of the stored vector
    libMesh::ParallelType type;
    /// The projection flag (whether or not it should be projected or zeroed)
    bool projections;
    /// The position of each variable for this vector (relative to the start of the data)
    std::map<std::string, std::size_t> variable_offset;
    /// The underlying vector (only valid during store, not used in load)
    const libMesh::NumericVector<libMesh::Number> * vector = nullptr;
  };

  /**
   * Represents a stored variable in restart
   */
  struct VariableHeader
  {
    bool operator==(const VariableHeader & other) const
    {
      return name == other.name && type == other.type && size == other.size &&
             variable == other.variable;
    }

    /// The name of the stored variable
    std::string name;
    /// The type of the stored variable
    libMesh::FEType type;
    /// The size of this variable's data
    std::size_t size = 0;
    /// The underlying variable (only valid during store, not used in load)
    const libMesh::Variable * variable = nullptr;
  };

  /**
   * Represents a stored system in restart
   */
  struct SystemHeader
  {
    bool operator==(const SystemHeader & other) const
    {
      return name == other.name && type == other.type && variables == other.variables &&
             vectors == other.vectors;
    }

    /// The name of the stored system
    std::string name;
    /// The type of the stored system
    std::string type;
    /// The stored variables in the system
    std::map<std::string, RestartableEquationSystems::VariableHeader> variables;
    /// The stored vectors in the system
    std::map<std::string, RestartableEquationSystems::VectorHeader> vectors;
    /// Special name for a vector that is the system solution vector
    static const std::string system_solution_name;
  };

  /**
   * Represents a stored EquationSystems in restart
   */
  struct EquationSystemsHeader
  {
    bool operator==(const EquationSystemsHeader & other) const { return systems == other.systems; }

    /// The stored systems in the equation systems
    std::map<std::string, RestartableEquationSystems::SystemHeader> systems;
    /// The total size of data for this EquationSystems
    std::size_t data_size = 0;
  };

  /**
   * Stores the EquationSystems to the given stream
   */
  void store(std::ostream & stream) const;
  /**
   * Loads the EquationSystems from the given stream
   */
  void load(std::istream & stream);

  /**
   * @returns The underyling EquationSystems
   */
  ///@{
  libMesh::EquationSystems & es() { return _es; }
  const libMesh::EquationSystems & es() const { return _es; }
  ///@}

  /**
   * Sets whether or not all vectors are to be loaded.
   *
   * By default, this is true. This means that all vectors
   * that do not currently exist in the system will be added
   * and loaded.
   *
   * Typically, we would want this to be false in the case
   * of restart.
   */
  void setLoadAllVectors(const bool load_all_vectors) { _load_all_vectors = load_all_vectors; }

  /// Checks whether variable was successfully restored from a restart file
  bool isVariableRestored(const std::string & system_name,
                          const std::string & vector_name,
                          const std::string & variable_name) const;

  /// Returns the set of variables that were loaded during the `load()` function. Each set of variables contain; [system name, vector name, variable name]
  const std::set<std::tuple<std::string, std::string, std::string>> & getLoadedVariables() const
  {
    return _loaded_variables;
  }

  /**
   * Represents a request to copy a single variable's solution data, loaded from a source
   * (checkpoint) EquationSystems, into a target System's vector during the next load().
   *
   * This is used to seed initial conditions from a checkpoint file (initial_from_file_var):
   * the source variable is looked up by name in the loaded header and its data is written
   * directly into a live System's solution vector. No systems/variables are added to this
   * object's own EquationSystems, so the dof indexing of the live systems is left untouched.
   */
  struct VariableCopy
  {
    /// Name of the source variable as stored in the loaded (checkpoint) data
    std::string source_variable;
    /// The target (live) system that owns the variable to write into
    const libMesh::System * to_system;
    /// The target (live) vector to write into (typically the system solution)
    libMesh::NumericVector<libMesh::Number> * to_vector;
    /// The target (live) variable to write into
    const libMesh::Variable * to_variable;
  };

  /**
   * Registers a variable copy to be performed during the next load(): the source variable
   * \p source_variable is located by name in the loaded data and its solution is written into
   * \p to_vector using the dof layout of \p to_variable within \p to_system.
   */
  void addVariableCopy(const std::string & source_variable,
                       const libMesh::System & to_system,
                       libMesh::NumericVector<libMesh::Number> & to_vector,
                       const libMesh::Variable & to_variable);

  /**
   * @returns Whether the variable copy targeting the variable \p to_variable_name in the system
   * \p to_system_name was performed during load()
   */
  bool wasVariableCopied(const std::string & to_system_name,
                         const std::string & to_variable_name) const;

  /// @returns The variable copies queued via addVariableCopy()
  const std::vector<VariableCopy> & getVariableCopies() const { return _variable_copies; }

private:
  /// Performs the queued variable copies (see addVariableCopy) from the loaded data into their
  /// targets. Called from load() once the header has been read.
  void performVariableCopies(std::istream & stream);

  /// Internal method for building the header struct
  EquationSystemsHeader
  buildHeader(const std::vector<const libMesh::DofObject *> & ordered_objects) const;

  /// Internal method for ordering the DofObjects by ID (elems and the nodes)
  std::vector<const libMesh::DofObject *> orderDofObjects() const;

  void restore(const SystemHeader & from_sys_header,
               const VectorHeader & from_vec_header,
               const VariableHeader & from_var_header,
               const libMesh::System & to_sys,
               libMesh::NumericVector<libMesh::Number> & to_vec,
               const libMesh::Variable & to_var,
               std::istream & stream);

  /// The underlying EquationSystems
  libMesh::EquationSystems _es;

  /// Whether or not to load _all_ of the vectors, including ones that haven't been added yet
  bool _load_all_vectors;
  /// The starting position for the vector data in the input stream
  std::size_t _loaded_stream_data_begin;
  /// The object ordering for this data
  std::vector<const libMesh::DofObject *> _loaded_ordered_objects;
  /// The loaded header
  EquationSystemsHeader _loaded_header;

  /// The variables that were loaded in load(); [system name, vector name, variable name]
  std::set<std::tuple<std::string, std::string, std::string>> _loaded_variables;

  /// Variable copies to perform during load() (see addVariableCopy)
  std::vector<VariableCopy> _variable_copies;

  /// The [target system name, target variable name] copies completed during the last load()
  std::set<std::pair<std::string, std::string>> _completed_variable_copies;
};

void dataStore(std::ostream & stream, RestartableEquationSystems & res, void *);
void dataLoad(std::istream & stream, RestartableEquationSystems & res, void *);

void dataStore(std::ostream & stream,
               RestartableEquationSystems::EquationSystemsHeader & header,
               void *);
void
dataLoad(std::istream & stream, RestartableEquationSystems::EquationSystemsHeader & header, void *);

void dataStore(std::ostream & stream, RestartableEquationSystems::SystemHeader & header, void *);
void dataLoad(std::istream & stream, RestartableEquationSystems::SystemHeader & header, void *);

void dataStore(std::ostream & stream, RestartableEquationSystems::VariableHeader & header, void *);
void dataLoad(std::istream & stream, RestartableEquationSystems::VariableHeader & header, void *);

void dataStore(std::ostream & stream, RestartableEquationSystems::VectorHeader & header, void *);
void dataLoad(std::istream & stream, RestartableEquationSystems::VectorHeader & header, void *);

void to_json(nlohmann::json & json, const RestartableEquationSystems & res);
