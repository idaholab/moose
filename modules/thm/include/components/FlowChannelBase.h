#pragma once

#include "GeometricalFlowComponent.h"

class FlowChannelBase;
class ClosuresBase;

template <>
InputParameters validParams<FlowChannelBase>();

/**
 * A base class for flow channels
 *
 * A flow channel is defined by its position, direction, length and area.
 */
class FlowChannelBase : public GeometricalFlowComponent
{
public:
  FlowChannelBase(const InputParameters & params);

  /// Type of convective heat transfer geometry
  enum EConvHeatTransGeom
  {
    PIPE,      ///< pipe geometry
    ROD_BUNDLE ///< rod bundle geometry
  };
  /// Map of convective heat transfer geometry type to enum
  static const std::map<std::string, EConvHeatTransGeom> _heat_transfer_geom_to_enum;

  virtual void buildMesh() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  /**
   * Gets a MooseEnum for convective heat transfer geometry type
   *
   * @param[in] name   default value
   * @returns MooseEnum for convective heat transfer geometry type
   */
  static MooseEnum getConvHeatTransGeometry(const std::string & name);

  // Flow channel specific interface ----
  virtual std::shared_ptr<const FlowModel> getFlowModel() const;

  /**
   * Get the name of the function describing the flow channel area
   *
   * @return The name of the function describing the flow channel area
   */
  const FunctionName & getAreaFunctionName() const { return _area_function; }

  const std::vector<unsigned int> & getNodeIDs() const;
  const std::vector<unsigned int> & getElementIDs() const;
  unsigned int getNodesetID() const;
  const BoundaryName & getNodesetName() const;
  unsigned int getSubdomainID() const;
  virtual bool isHorizontal() const { return _is_horizontal; }

  /**
   * Gets heat transfer geometry
   */
  EConvHeatTransGeom getHeatTransferGeometry() const { return _HT_geometry; }

  /**
   * Gets temperature mode flag
   */
  bool getTemperatureMode() const { return _temperature_mode; }

  /**
   * Gets the number of heat transfer connections
   */
  unsigned int getNumberOfHeatTransferConnections() const { return _n_heat_transfer_connections; }

  /**
   * Gets heated perimeter names for connected heat transfers
   */
  std::vector<VariableName> getHeatedPerimeterNames() const { return _P_hf_names; }

  /**
   * Gets wall temperature names for connected heat transfers
   */
  std::vector<VariableName> getWallTemperatureNames() const { return _T_wall_names; }

  /**
   * Adds the name of a heat transfer component to the flow channel's list.
   *
   * This function is called from a heat transfer component to add its name to
   * the flow channel's list, so that the flow channel can communicate with it, such as for
   * determining variable names used in averages of heat transfer variables.
   *
   * @param[in] ht_name  name of the heat transfer component
   */
  void addHeatTransferName(const std::string & ht_name) const;

  /**
   * Gets suffix to add to heat-transfer-related names in a heat transfer component
   *
   * This function is called from a heat transfer component to determine how to
   * name its variables. If the flow channel has only one heat transfer coefficient, for
   * example, then no suffix is returned. This function must be called after all
   * of the heat transfer component names have been added to the flow channel's list
   * (via \c addHeatTransferName()).
   *
   * @param[in] ht_name  name of the heat transfer component
   * @return suffix to add to heat transfer variable names
   */
  std::string getHeatTransferNamesSuffix(const std::string & ht_name) const;

  /**
   * Get the used closures object
   *
   * @return The closures object
   */
  std::shared_ptr<ClosuresBase> getClosures() const { return _closures; }

protected:
  virtual std::shared_ptr<FlowModel> buildFlowModel() = 0;
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  virtual std::shared_ptr<ClosuresBase> buildClosures();

  virtual void buildMeshNodes();

  /**
   * Adds objects which are common for single- and two-phase flow
   */
  virtual void addCommonObjects();

  virtual void setupDh();

  /**
   * Populates heat connection variable names lists
   */
  virtual void getHeatTransferVariableNames();

  /// The flow model used by this flow channel
  std::shared_ptr<FlowModel> _flow_model;

  /// Function describing the flow channel area
  const FunctionName & _area_function;

  /// The name of used closures
  const std::string & _closures_name;

  /// Closures object
  std::shared_ptr<ClosuresBase> _closures;

  const bool & _pipe_pars_transferred;

  /// True if the user provided a function describing the hydraulic diameter of the flow channel
  bool _has_Dh;
  /// Function describing the hydraulic diameter
  FunctionName _Dh_function;

  /// Roughness of flow channel surface, [m]
  const Real & _roughness;

  /// Convective Heat transfer geometry
  EConvHeatTransGeom _HT_geometry;
  /// Pitch to diameter ratio for parallel bundle heat transfer
  const Real & _PoD;
  /// True if user provides PoD
  bool _has_PoD;

  /// Subdomain id this flow channel defined
  unsigned int _subdomain_id;
  /// Nodeset id for all flow channel nodes
  unsigned int _nodeset_id;
  /// Nodeset name for all flow channel nodes
  BoundaryName _nodeset_name;

  /// Nodes ids of this flow channel component
  std::vector<unsigned int> _node_ids;
  /// Elements ids of this flow channel component
  std::vector<unsigned int> _elem_ids;

  /// Flag that flow channel is "horizontal"; else "vertical". The flow channel
  /// is considered "horizontal" if the angle between the flow channel orientation
  /// vector and gravity vector is between 45 degrees and 135 degrees.
  const bool _is_horizontal;

  /// The name of the user object that will set up stabilization
  UserObjectName _stabilization_uo_name;

  /// True if there is one or more sources specified by wall temperature
  bool _temperature_mode;
  /// Names of the heat transfer components connected to this component
  mutable std::vector<std::string> _heat_transfer_names;
  /// Number of connected heat transfer components
  mutable unsigned int _n_heat_transfer_connections;

  /// 1-phase wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_1phase_names;
  /// liquid wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_liquid_names;
  /// vapor wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_vapor_names;
  /// heated perimeter names for connected heat transfers
  std::vector<VariableName> _P_hf_names;
  /// wall temperature names for connected heat transfers
  std::vector<VariableName> _T_wall_names;
  /// wall heat flux names for connected heat transfers
  std::vector<MaterialPropertyName> _q_wall_names;
};

namespace THM
{
template <>
FlowChannelBase::EConvHeatTransGeom stringToEnum(const std::string & s);
}
