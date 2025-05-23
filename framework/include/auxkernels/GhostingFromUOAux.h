//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

#include "libmesh/ghosting_functor.h"

class GhostingUserObject;

/**
 * Outputs to an auxiliary variable the ghosting of a GhostingUserObject
 */
class GhostingFromUOAux : public AuxKernel
{
public:
  static InputParameters validParams();

  GhostingFromUOAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  virtual void precalculateValue() override;

  /// The PID to show the ghosting for
  processor_id_type _pid;

  /// The type of ghosting functor to get
  Moose::RelationshipManagerType _rm_type;

  /// Whether or not to include local elements in the display field
  bool _include_local;

  /// precalculated element value
  Real _value;

  /// The reference to the ghosting user object
  const GhostingUserObject & _ghost_uo;

  /// Ghosted elems
  libMesh::GhostingFunctor::map_type _ghosted_elems;
};
