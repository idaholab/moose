//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"
#include "MooseUtils.h"

class FluidProperties;
class Simulation;

#define registerComponent(name) registerObject(name)
#define registerNamedComponent(obj, name) registerNamedObject(obj, name)
#define registerControl(name) registerObject(name)
#define registerNamedControl(obj, name) registerNamedObject(obj, name)

#define registerFlowModel(id, class_name)                                                          \
  _flow_model_map.insert(std::pair<THM::FlowModelID, std::string>(id, stringifyName(class_name)));

namespace THM
{

typedef unsigned int FlowModelID;

/**
 * Register a new flow mode type and return its ID
 *
 * @return ID of the newly registered
 */
FlowModelID registerFlowModelID();

extern FlowModelID FM_INVALID;
extern FlowModelID FM_SINGLE_PHASE;
extern FlowModelID FM_TWO_PHASE;
extern FlowModelID FM_TWO_PHASE_NCG;

// This is the upper limit on variable length given by exodusII
static const size_t MAX_VARIABLE_LENGTH = 31;

}

class ThermalHydraulicsApp : public MooseApp
{
public:
  ThermalHydraulicsApp(InputParameters parameters);
  virtual ~ThermalHydraulicsApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

  /**
   * Deprecated Methods
   */
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);

  /**
   * Get the class name of a flow model corresponding to the flow model ID
   *
   * @param closure_name The name of the closure type
   * @return The class name of a material that computes the flow regime maps
   */
  const std::string & getFlowModelClassName(const THM::FlowModelID & flow_model_id);

public:
  /// Map from flow model ID to flow model instance
  static std::map<THM::FlowModelID, std::string> _flow_model_map;

  static InputParameters validParams();
};
