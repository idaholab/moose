/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMMODEL_H
#define STATESIMMODEL_H

#include <string>
#include <array>
#include "StateSimAllEvents.h"
#include "StateSimAllStates.h"
#include "StateSimAllDiagrams.h"
#include "StateSimAllVariables.h"
#include "StateSimAllActions.h"

//Forward Declarations
class StateExternalCouplingUO;

/**
 Object to load and run and manage a state simulation model
 */
class StateSimModel
{
public:
  StateSimAllEvents _time_events;
  StateSimAllEvents _condition_events;
  StateSimAllStates _states;
  StateSimAllDiagrams _diagrams;
  StateSimAllVariables _variables;
  StateSimAllActions _actions;
  const TimespanH _max_sim_time;

  /**
   * Main constructor to build a StateSim model.
   * @param max_sim_time - maximum total simulation time before forced stop of the simulation
   * @param ext_coupling_refs - user object for external item reference coupling.
   */
  StateSimModel(const TimespanH & max_sim_time, const StateExternalCouplingUO & ext_coupling_refs);

  /**
   * nextID - called by the StateSim object being created to get a bitset id.
   * @param obj_type - the type of StateSim object to make an ID for.
   */
  int nextID(STATESIM_TYPE obj_type);

  bool test();

private:
  std::array<int, (static_cast<size_t>(STATESIM_TYPE::LAST) +1)> _next_id;
};

#endif

