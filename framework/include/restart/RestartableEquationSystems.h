//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

private:
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
