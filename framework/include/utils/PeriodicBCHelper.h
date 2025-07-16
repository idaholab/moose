//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

#include "libmesh/periodic_boundaries.h"

class Action;
class FEProblemBase;
class InputParameters;

namespace Moose
{
/**
 * Helper class for setting up periodic boundary conditions via an Action.
 *
 * Determines the PeriodicBoundaries given the parameters and sets up
 * geometric ghosting. Does not handle algebraic ghosting.
 */
class PeriodicBCHelper
{
public:
  static InputParameters validParams();

  PeriodicBCHelper(const Action & action);

protected:
  /**
   * Checks the validity of the periodic boundary condition parameters.
   */
  void checkPeriodicParams() const;

  /**
   * Sets up the periodic boundaries.
   *
   * Should be called by the owning Action. Once called, the PeriodicBoundaries
   * object will be available via getPeriodicBoundaries().
   *
   * onSetupPeriodicBoundary() will be called on each periodic boundary
   * added to enable the derived class to extend.
   */
  void setupPeriodicBoundaries(FEProblemBase & problem);

  /**
   * Entry-point for derived actions to extend the addition of a periodic boundary.
   */
  virtual void onSetupPeriodicBoundary(libMesh::PeriodicBoundaryBase & /* p */) {};

  /**
   * Get the PeriodicBoundaries map produced in setupPeriodicBoundaries().
   */
  const libMesh::PeriodicBoundaries & getPeriodicBoundaries() const { return _periodic_boundaries; }

private:
  /**
   * Internal helper for adding a periodic boundary.
   */
  void addPeriodicBoundary(std::unique_ptr<libMesh::PeriodicBoundaryBase> /* p */);

  /**
   * Internal method for setting up periodic boundaries via the "auto_direction" param.
   */
  void setupAutoPeriodicBoundaries(FEProblemBase & problem);
  /**
   * Internal method for setting up manual periodic boundaries via the
   * "translation" and "transform_func" params.
   */
  void setupManualPeriodicBoundaries(FEProblemBase & problem);

  /**
   * Helper for throwing a parameter error.
   *
   * Needed because this class can be used by both Actions and MooseObjectActions.
   *
   * In the future, we should be able to just call _params.paramError(),
   * but that's not in yet.
   */
  void paramError(const std::string & param_name, const std::string & message) const;

  /**
   * Internal method for getting the parameters by the owned action.
   *
   * Enables the use of the MOOSE object parameters if the owner is a
   * MooseObjectAction.
   */
  const InputParameters & getParams() const;

  /// The owning Action
  const Action & _action;
  /// The parameters used to create the periodic boundary
  const InputParameters & _params;
  /// The PeriodicBoundaries map, filled in setupPeriodicBoundaries()
  libMesh::PeriodicBoundaries _periodic_boundaries;
};
}
