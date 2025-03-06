
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryCondition.h"
#include "AbaqusUELMesh.h"

/**
 * Implements a simple constant Dashpot BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class AbaqusEssentialBC : public BoundaryCondition
{
public:
  static InputParameters validParams();

  AbaqusEssentialBC(const InputParameters & parameters);

  virtual void timestepSetup();

protected:
  AbaqusUELMesh * _uel_mesh;

  /// current node being processed
  const Node * const & _current_node;

  // prepare BC data in a map for easy retrieval
  std::unordered_map<Abaqus::Index, std::vector<std::pair<MooseVariableField<Real> *, Real>>>
      _bc_data;
};
