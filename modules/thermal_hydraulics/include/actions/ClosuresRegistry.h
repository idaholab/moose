//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "Logger.h"

class ClosuresBase;

/**
 * Problem-agnostic, name-keyed registry of Closures objects (see ClosuresBase), auto-created via
 * findOrCreate() the same way THMVariableCoordinator is - so a [Closures] block can be looked up
 * by name from either a classic THM Component (via Simulation::addClosures()/getClosures(),
 * which now just delegate here) or a native ActionComponent's Physics (e.g.
 * FlowChannel1PhasePhysics), without needing a THMProblem to exist.
 *
 * Not meant to be declared by the user in an input file.
 */
class ClosuresRegistry : public Action
{
public:
  static InputParameters validParams();
  ClosuresRegistry(const InputParameters & params);

  virtual void act() override {}

  /// Builds a Closures object of the given type/name and registers it under that name
  void addClosures(const std::string & type, const std::string & name, InputParameters params);
  /// Whether a Closures object with the given name has been registered
  bool hasClosures(const std::string & name) const;
  /// Returns the Closures object registered under the given name
  std::shared_ptr<ClosuresBase> getClosures(const std::string & name) const;

  /// The logger shared by every Closures object this registry builds (mirrors classic THM's
  /// Simulation::_log) - emit with emitLoggedMessages() once all closures checks have run
  Logger & getLogger() { return _log; }
  /// Emits (as mooseError/mooseWarning) any messages logged by a Closures object's
  /// checkFlowChannel()/checkHeatTransfer() so far
  void emitLoggedMessages() const;

  /// Finds this simulation's registry, creating and registering it (via \p action_factory) with
  /// \p awh if it does not already exist
  static ClosuresRegistry & findOrCreate(ActionWarehouse & awh, ActionFactory & action_factory);

private:
  std::map<std::string, std::shared_ptr<ClosuresBase>> _closures_by_name;

  /// Shared logger for closures objects built by this registry
  Logger _log;
};
