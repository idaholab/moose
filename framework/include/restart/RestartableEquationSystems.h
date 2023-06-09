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
  struct VariableHeader
  {
    std::string name;
    unsigned int number;
    libMesh::FEType type;
  };

  /**
   * Represents a stored system in restart
   */
  struct SystemHeader
  {
    std::string name;
    std::string type;
    unsigned int number;
    std::map<std::string, RestartableEquationSystems::VariableHeader> variables;
    std::vector<std::string> vectors;
  };

  /**
   * Represents a stored EquationSystems in restart
   */
  struct EquationSystemsHeader
  {
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
  std::vector<const DofObject *> orderDofObjects() const;

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
