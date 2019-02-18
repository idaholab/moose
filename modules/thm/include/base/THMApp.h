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

#define registerClosureClass(closure_map, closure_name, class_name)                                \
  if (closure_map.find(MooseUtils::toLower(closure_name)) != closure_map.end() &&                  \
      closure_map[MooseUtils::toLower(closure_name)] != stringifyName(class_name))                 \
    mooseError("Attempting to register the closure '" + MooseUtils::toLower(closure_name) +        \
               "' with the class '" + stringifyName(class_name) +                                  \
               "', but it has already been registered with the class '" +                          \
               closure_map[MooseUtils::toLower(closure_name)] + "'");                              \
  else                                                                                             \
    closure_map.insert(std::pair<std::string, std::string>(MooseUtils::toLower(closure_name),      \
                                                           stringifyName(class_name)));

#define registerWallHeatTransferCoefficentSinglePhaseAux(closure_name, class_name)                 \
  registerClosureClass(_whtc_3eqn_name_map, closure_name, class_name);

#define registerWallHeatTransferCoefficentTwoPhaseAux(closure_name, class_name)                    \
  registerClosureClass(_whtc_7eqn_name_map, closure_name, class_name);

#define registerWallFrictionCoefficentSinglePhaseMaterial(closure_name, class_name)                \
  registerClosureClass(_wfc_3eqn_name_map, closure_name, class_name);

#define registerWallFrictionCoefficentTwoPhaseMaterial(closure_name, class_name)                   \
  registerClosureClass(_wfc_7eqn_name_map, closure_name, class_name);

#define registerCriticalHeatFluxTable(table_name, class_name)                                      \
  _chf_name_map.insert(std::pair<std::string, std::string>(MooseUtils::toLower(table_name),        \
                                                           stringifyName(class_name)));

#define registerInterfacialHeatTransferMaterial(closure_name, class_name)                          \
  registerClosureClass(_iht_name_map, closure_name, class_name);

#define registerInterfacialFrictionCoefficientMaterial(closure_name, class_name)                   \
  registerClosureClass(_ifc_name_map, closure_name, class_name);

#define registerSpecificInterfacialAreaMaterial(closure_name, class_name)                          \
  registerClosureClass(_sia_name_map, closure_name, class_name);

#define registerFlowRegimeMapMaterial(closure_name, class_name)                                    \
  registerClosureClass(_frm_name_map, closure_name, class_name);

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
   * Get the set of all available closures
   *
   * @return The set of all registered closures
   */
  static const std::set<std::string> & closureTypes() { return _closure_types; }

  /**
   * Get the default closure type
   *
   * @return The name of the default closure
   */
  static const std::string & defaultClosureType() { return _default_closure_type; }

  /**
   * Checks that a closure map has a registered entry before returning the registered name
   *
   * @param[in] closure_map   map of the closure name to the relevant class name
   * @param[in] closure_name  name of the closures
   * @param[in] description   description of the map
   */
  const std::string & getClosureMapEntry(const std::map<std::string, std::string> & closure_map,
                                         const std::string & closure_name,
                                         const std::string & description) const;

  /**
   * Get the class name of an auxkernel that computes the wall heat transfer coefficient for single
   * phase
   *
   * @param closure_name The name of the closure type
   * @return The class name of an auxkernel that computes the wall heat transfer coefficient
   */
  const std::string & getWallHeatTransferCoefficent3EqnClassName(const std::string & closure_name)
  {
    return getClosureMapEntry(
        _whtc_3eqn_name_map, closure_name, "1-phase wall heat transfer coefficient");
  }

  /**
   * Get the class name of a material that computes the wall friction coefficient for single
   * phase
   *
   * @param closure_name The name of the closure type
   * @return The class name of a material that computes the wall friction coefficient
   */
  const std::string & getWallFrictionCoefficent3EqnClassName(const std::string & closure_name)
  {
    return getClosureMapEntry(
        _wfc_3eqn_name_map, closure_name, "1-phase wall friction coefficient");
  }

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

  /**
   * Get the class name of a material that computes the specific interfacial area
   *
   * @param closure_name The name of the closure type
   * @return The class name of a material that computes the specific interfacial area
   */
  const std::string & getSpecificInterfacialAreaMaterialClassName(const std::string & closure_name)
  {
    return getClosureMapEntry(_sia_name_map, closure_name, "specific interfacial area");
  }

  /**
   * Get the class name of an auxkernel that computes the wall heat transfer coefficient for two
   * phase
   *
   * @param closure_name The name of the closure type
   * @return The class name of an auxkernel that computes the wall heat transfer coefficient
   */
  const std::string & getWallHeatTransferCoefficent7EqnClassName(const std::string & closure_name)
  {
    return getClosureMapEntry(
        _whtc_7eqn_name_map, closure_name, "2-phase wall heat transfer coefficient");
  }

  /**
   * Get the class name of a material that computes the wall friction coefficient for two
   * phase
   *
   * @param closure_name The name of the closure type
   * @return The class name of a material that computes the wall friction coefficient
   */
  const std::string & getWallFrictionCoefficent7EqnClassName(const std::string & closure_name)
  {
    return getClosureMapEntry(
        _wfc_7eqn_name_map, closure_name, "2-phase wall friction coefficient");
  }

  /**
   * Get the class name of a material that computes the interfacial heat transfer coefficients
   *
   * @param closure_name The name of the closure type
   * @return The class name of a material that computes the interfacial heat transfer coefficients
   */
  const std::string & getInterfacialHeatTransferMaterialClassName(const std::string & closure_name)
  {
    return getClosureMapEntry(_iht_name_map, closure_name, "interfacial heat transfer coefficient");
  }

  /**
   * Get the class name of a material that computes the interfacial friction coefficient
   *
   * @param closure_name The name of the closure type
   * @return The class name of a material that computes the interfacial friction coefficients
   */
  const std::string &
  getInterfacialFrictionCoefficientMaterialClassName(const std::string & closure_name)
  {
    return getClosureMapEntry(_ifc_name_map, closure_name, "interfacial friction coefficient");
  }

  /**
   * Get the class name of a material that computes the flow regime maps
   *
   * @param closure_name The name of the closure type
   * @return The class name of a material that computes the flow regime maps
   */
  const std::string & getFlowRegimeMapMaterialClassName(const std::string & closure_name)
  {
    return getClosureMapEntry(_frm_name_map, closure_name, "flow regime map");
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
   * Register a new closure type
   *
   * @param closure_type The name for the new closures. The name is visible to users.
   * @param is_default True if this should be the default closure type. The last call claiming
   * default will be the default.
   */
  static void registerClosureType(const std::string & closure_type, bool is_default = false);

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

  /// Map from flow model ID to flow model instance
  static std::map<THM::FlowModelID, std::string> _flow_model_map;

  /// The set of closure types
  static std::set<std::string> _closure_types;
  /// The default closure type
  static std::string _default_closure_type;

  /// The map from closure name to a wall heat transfer coefficient auxkernel class name (1-phase)
  static std::map<std::string, std::string> _whtc_3eqn_name_map;
  /// The map from closure name to a wall friction coefficient material class name (1-phase)
  static std::map<std::string, std::string> _wfc_3eqn_name_map;
  /// The map from closure name to a wall heat transfer coefficient auxkernel class name (2-phase)
  static std::map<std::string, std::string> _whtc_7eqn_name_map;
  /// The map from closure name to a wall friction coefficient material class name (2-phase)
  static std::map<std::string, std::string> _wfc_7eqn_name_map;
  /// The map from closure name to a interfacial heat transfer material class name
  static std::map<std::string, std::string> _iht_name_map;
  /// The map from closure name to a interfacial friction material class name
  static std::map<std::string, std::string> _ifc_name_map;
  /// The map from closure name to a specific interfacial area material class name
  static std::map<std::string, std::string> _sia_name_map;
  /// The map from closure name to a flow regime map material class name
  static std::map<std::string, std::string> _frm_name_map;
  /// The set of critical heat flux table types
  static std::set<std::string> _chf_table_types;
  /// The default critical heat flux table type
  static std::string _default_chf_table_type;
  /// The map from CHF table name to a user object class name
  static std::map<std::string, std::string> _chf_name_map;

  [[noreturn]] void raiseFlowModelError(const FluidProperties & fp, const std::string & mbdf);
};

#endif /* THMAPP_H */
