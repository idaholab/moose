#ifndef THMAPP_H
#define THMAPP_H

#include "MooseApp.h"
#include "Logger.h"
#include "MooseUtils.h"

class THMApp;
class FluidProperties;
class Simulation;

#define registerComponent(name) registerObject(name)
#define registerNamedComponent(obj, name) registerNamedObject(obj, name)
#define registerControl(name) registerObject(name)
#define registerNamedControl(obj, name) registerNamedObject(obj, name)

#define registerFlowModel(id, class_name)                                                          \
  _flow_model_map.insert(std::pair<THM::FlowModelID, std::string>(id, stringifyName(class_name)));

#define registerCriticalHeatFluxTable(table_name, class_name)                                      \
  _chf_name_map.insert(std::pair<std::string, std::string>(MooseUtils::toLower(table_name),        \
                                                           stringifyName(class_name)));

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

/// special nodeset id for nodes on boundaries
extern boundary_id_type bnd_nodeset_id;
}

template <>
InputParameters validParams<THMApp>();

class THMApp : public MooseApp
{
public:
  THMApp(InputParameters parameters);
  virtual ~THMApp();

  virtual void setupOptions() override;

  /**
   * Gets the class name of the closures corresponding to the flow model and user option
   *
   * @param[in] closures_option   Closures option
   * @param[in] flow_model_id     Flow model ID
   */
  const std::string & getClosuresClassName(const std::string & closures_option,
                                           const THM::FlowModelID & flow_model_id) const;

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

  /**
   * Returns a flow model ID for the specified fluid properties
   *
   * @return The flow model ID corresponding to the given fluid properties
   * @param fp FluidProperties class to get the model ID for
   */
  virtual const THM::FlowModelID & getFlowModelID(const FluidProperties & fp);

  /**
   * Get the class name of a flow model corresponding to the flow model ID
   *
   * @param closure_name The name of the closure type
   * @return The class name of a material that computes the flow regime maps
   */
  const std::string & getFlowModelClassName(const THM::FlowModelID & flow_model_id);

  /**
   * Get the set of all available critical heat flux tables
   *
   * @return The set of all registered critical heat flux tables
   */
  static const std::set<std::string> & criticalHeatFluxTableTypes() { return _chf_table_types; }

  /**
   * Get the default critical heat flux table
   *
   * @return The name of the default critical heat flux table
   */
  static const std::string & defaultCriticalHeatFluxTableType() { return _default_chf_table_type; }

  /**
   * Get the class name of a user object that represents the CHF table
   *
   * @param chf_name The name of the critical heat flux lookup table
   * @return The class name of a user object that represents the CHF table
   */
  const std::string & getCriticalHeatFluxTableClassName(const std::string & chf_name)
  {
    return _chf_name_map[MooseUtils::toLower(chf_name)];
  }

  Logger & log() { return _log; }
  virtual bool checkJacobian() { return _check_jacobian; }

protected:
  /**
   * Build the simulation class
   */
  virtual void buildSimulation();

  void build();

  /**
   * Registers a closures option
   *
   * @param[in] closures_option   Closures option string to register
   * @param[in] closures_name     Closures class name
   * @param[in] flow_model_id     Flow model ID
   */
  static void registerClosuresOption(const std::string & closures_option,
                                     const std::string & class_name,
                                     const THM::FlowModelID & flow_model_id);

  /**
   * Register a new critical heat flux table
   *
   * @param chf_table_type The name for the new critical heat flux table. The name is visible
   * to users.
   * @param is_default True if this should be the default type. The last call claiming default will
   * be the default.
   */
  static void registerCriticalHeatFluxTableType(const std::string & chf_table_type,
                                                bool is_default = false);

  Logger _log;
  Simulation * _sim;
  bool _check_jacobian;

  /// Map from flow model ID to map of closures option to its class
  static std::map<THM::FlowModelID, std::map<std::string, std::string>> _closures_class_names_map;

  /// Map from flow model ID to flow model instance
  static std::map<THM::FlowModelID, std::string> _flow_model_map;

  /// The set of critical heat flux table types
  static std::set<std::string> _chf_table_types;
  /// The default critical heat flux table type
  static std::string _default_chf_table_type;
  /// The map from CHF table name to a user object class name
  static std::map<std::string, std::string> _chf_name_map;

  [[noreturn]] void raiseFlowModelError(const FluidProperties & fp, const std::string & mbdf);
};

#endif /* THMAPP_H */
