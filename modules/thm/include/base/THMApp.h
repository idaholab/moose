#ifndef THMAPP_H
#define THMAPP_H

#include "MooseApp.h"
#include "Logger.h"

class THMApp;
class FluidProperties;

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
}

template <>
InputParameters validParams<THMApp>();

class THMApp : public MooseApp
{
public:
  THMApp(InputParameters parameters);

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

  /**
   * Returns a flow model ID for the specified fluid properties
   *
   * @return The flow model ID corresponding to the given fluid properties
   * @param fp FluidProperties class to get the model ID for
   */
  virtual const THM::FlowModelID & getFlowModelID(const FluidProperties & fp);

  Logger & log() { return _log; }

protected:
  Logger _log;

  /// Map from flow model ID to flow model instance
  static std::map<THM::FlowModelID, std::string> _flow_model_map;

  [[noreturn]] void raiseFlowModelError(const FluidProperties & fp, const std::string & mbdf);
};

#endif /* THMAPP_H */
