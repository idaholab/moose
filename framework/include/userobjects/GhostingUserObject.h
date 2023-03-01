//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

#include "libmesh/ghosting_functor.h"

#include <unordered_map>

class MooseMesh;
class NonlinearSystemBase;

/**
 * This object loops over all of the underlying ghosting functors added by libMesh or MOOSE through
 * RelationshipManagers to display the effective ghosting for a particular simulation. Normally this
 * information is output through several AuxVariables.
 */
class GhostingUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  GhostingUserObject(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  virtual void meshChanged() override;

  Real getElementalValue(const Elem * elem,
                         Moose::RelationshipManagerType rm_type,
                         processor_id_type pid) const;

private:
  /// The PID to show the ghosting for
  std::vector<processor_id_type> _pids;

  /// Ghost Functor maps
  /// Dimension one:   Map type (Geometric, Algebraic)
  /// Dimension two:   Proc ID -> Map
  /// Dimension three: Elem Ptr -> Coupling Matrix
  std::vector<std::unordered_map<processor_id_type, libMesh::GhostingFunctor::map_type>> _maps;

  MooseMesh & _mesh;
  NonlinearSystemBase & _nl;
};
