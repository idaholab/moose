//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "libmesh/fe_type.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/equation_systems.h"
#include "libmesh/exodusII_io.h"
#include "MooseTypes.h"

namespace libMesh
{
class System;
class DofMap;
}

/**
 * Utility function to read a single variable off an Exodus mesh for optimization problem
 * This class will read initial conditions and bounds from an exodus file and make sure they have
 * the same order in the parameter reporter between the main and subapp when using
 * ParameterMeshOptimization.
 */

class ReadExodusMeshVars
{
public:
  ReadExodusMeshVars(const libMesh::FEType & param_type,
                     const std::string & exodus_mesh,
                     const std::string var_name);
  /**
   * Initializes parameter data and sets bounds in the main optmiization application
   * getParameterValues is only used by ParameterMeshOptimization
   * @param timestep  timestep to read variable off mesh
   * @return vector of variables read off mesh at timestep
   */
  std::vector<Real> getParameterValues(const unsigned int timestep) const;

private:
  libMesh::Parallel::Communicator _communicator;
  libMesh::ReplicatedMesh _mesh;
  std::unique_ptr<libMesh::EquationSystems> _eq;
  libMesh::System * _sys;
  std::unique_ptr<libMesh::ExodusII_IO> _exodusII_io;

  /// variable name read from Exodus mesh
  const std::string _var_name;
};
