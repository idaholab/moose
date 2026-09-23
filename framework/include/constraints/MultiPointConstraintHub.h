//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/system.h"

class NonlinearSystemBase;
class SystemBase;

/**
 * The single libMesh constraint object of a system, which hands the rows of every active
 * Constraint of a nonlinear system that provides them to the DofMap of the system it constrains.
 *
 * libMesh admits one constraint object per System and calls it through System::user_constrain()
 * every time the constraints of the system are rebuilt, so the rows come back by themselves after
 * a mesh change or a repartitioning.
 *
 * A problem with displacements carries a second, displaced copy of the nonlinear system, and MOOSE
 * assembles every object with 'use_displaced_mesh = true' through the Assembly of that copy, which
 * reduces its element vectors and matrices with the constraints of the displaced DofMap. The rows
 * must therefore reach both DofMaps, so one hub is built per system to constrain, each of them
 * serving the constraints of the same nonlinear system. The two systems share their degree of
 * freedom numbering, which is what lets MOOSE assemble displaced contributions into the vectors
 * and matrices of the undisplaced system, so both hubs add the very same rows.
 */
class MultiPointConstraintHub : public libMesh::System::Constraint
{
public:
  /**
   * @param sys The nonlinear system holding the constraints to apply
   * @param constrained_system The system whose DofMap receives the rows. It is either \p sys itself
   * or its displaced counterpart
   */
  MultiPointConstraintHub(NonlinearSystemBase & sys, SystemBase & constrained_system);

  virtual void constrain() override;

  /**
   * Rebuild the constraints of the constrained system, which calls constrain() again and so
   * replaces the rows of every row provider with the rows it emits now. Only the coefficients of
   * the rows may change: the sparsity pattern of the matrices was built once, from the rows of the
   * first build, so tying a different set of degrees of freedom remains a mesh change.
   */
  void reinit();

private:
  /// The nonlinear system whose constraints this hub applies
  NonlinearSystemBase & _sys;

  /// The system whose DofMap receives the rows
  SystemBase & _constrained_sys;
};
