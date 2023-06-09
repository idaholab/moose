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
  RestartableEquationSystems(libMesh::MeshBase & mesh, const bool skip_additional_vectors);

  /**
   * Represents a stored variable in restart
   */
  struct VectorHeader
  {
    /// The name of the stored vector
    std::string name;
    /// The stored vector (only valid when _storing_, not when loading)
    const libMesh::NumericVector<Number> * vec = nullptr;
    /// The to vector (only valid when _loading_, not when storing)
    libMesh::NumericVector<Number> * to_vec = nullptr;
  };

  /**
   * Represents a stored variable in restart
   */
  struct VariableHeader
  {
    /// The name of the stored variable
    std::string name;
    /// The type of the stored variable
    libMesh::FEType type;
    /// The stored variable (only valid when _storing_, not when loading)
    const libMesh::Variable * var = nullptr;
    /// The to variable (only valid when _loading_, not when storing)
    const libMesh::Variable * to_var = nullptr;
  };

  /**
   * Represents a stored system in restart
   */
  struct SystemHeader
  {
    /// The name of the stored system
    std::string name;
    /// The type of the stored system
    std::string type;
    /// The stored system (only valid when _storing_, not when loading)
    const libMesh::System * sys = nullptr;
    /// The to system (only valid when _loading_, not when storing)
    libMesh::System * to_sys = nullptr;
    /// The stored variables in the system
    std::map<std::string, RestartableEquationSystems::VariableHeader> variables;
    /// The stored vectors in the system
    std::vector<RestartableEquationSystems::VectorHeader> vectors;
  };

  /**
   * Represents a stored EquationSystems in restart
   */
  struct EquationSystemsHeader
  {
    /// The stored systems in the equation systems
    std::map<std::string, RestartableEquationSystems::SystemHeader> systems;
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

private:
  /// Internal method for building the header struct
  EquationSystemsHeader buildHeader() const;

  /// Internal method for ordering the DofObjects by ID (elems and the nodes)
  std::vector<const libMesh::DofObject *> orderDofObjects() const;

  /// The underlying EquationSystems
  libMesh::EquationSystems _es;

  /// Whether or not to skip the loading of additional (non-solution) vectors
  const bool _skip_additional_vectors;
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
