//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Component1D.h"
#include "GravityInterface.h"

class ClosuresBase;

/**
 * A base class for flow channels
 *
 * A flow channel is defined by its position, direction, length and area.
 */
class FlowChannelBase : public Component1D, public GravityInterface
{
public:
  FlowChannelBase(const InputParameters & params);

  /// Type of convective heat transfer geometry
  enum EConvHeatTransGeom
  {
    PIPE,          ///< pipe geometry
    ROD_BUNDLE,    ///< square array rod bundle geometry
    HEX_ROD_BUNDLE ///< hexagonal array rod bundle geometry
  };
  /// Pipe type
  enum EPipeType
  {
    STRAIGHT,
    CURVED,
    DOWNCOMER
  };
  /// Pipe location
  enum EPipeLocation
  {
    INTERIOR,
    EDGE,
    CORNER
  };
  /// Map of convective heat transfer geometry type to enum
  static const std::map<std::string, EConvHeatTransGeom> _heat_transfer_geom_to_enum;
  /// Map of pipe type to enum
  static const std::map<std::string, EPipeType> _pipe_type_to_enum;
  /// Map of pipe location to enum
  static const std::map<std::string, EPipeLocation> _pipe_location_to_enum;

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  /**
   * Gets the gravity angle for this component
   *
   * @return gravity angle for this component
   */
  virtual const Real & getGravityAngle() const { return _gravity_angle; }

  /**
   * Gets the name of the fluid properties user object for this component
   */
  const UserObjectName & getFluidPropertiesName() const { return _fp_name; }

  /**
   * Gets the flow model ID
   */
  virtual const THM::FlowModelID & getFlowModelID() const = 0;

  /**
   * Gets a MooseEnum for convective heat transfer geometry type
   *
   * @param[in] name   default value
   * @returns MooseEnum for convective heat transfer geometry type
   */
  static MooseEnum getConvHeatTransGeometry(const std::string & name);

  /**
   * Gets a MooseEnum for pipe type
   *
   * @param[in] name   default value
   * @returns MooseEnum for pipe type
   */
  static MooseEnum getPipeType(const std::string & name);

  /**
   * Gets a MooseEnum for pipe location
   *
   * @param[in] name   default value
   * @returns MooseEnum for pipe location
   */
  static MooseEnum getPipeLocation(const std::string & name);

  // Flow channel specific interface ----
  virtual std::shared_ptr<const FlowModel> getFlowModel() const;

  /**
   * Get the name of the function describing the flow channel area
   *
   * @return The name of the function describing the flow channel area
   */
  const FunctionName & getAreaFunctionName() const;

  /**
   * Creates the area function if needed and then stores the name
   */
  virtual FunctionName createAreaFunctionAndGetName();

  /**
   * Gets heat transfer geometry
   */
  EConvHeatTransGeom getHeatTransferGeometry() const { return _HT_geometry; }

  /**
   * Gets the pipe location
   */
  EPipeLocation getPipeLocation() const { return _pipe_location; }

  /**
   * Gets the names of all connected heat transfer components
   */
  std::vector<std::string> getHeatTransferNames() const;

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
  std::vector<MaterialPropertyName> getWallTemperatureMatNames() const { return _T_wall_mat_names; }

  /**
   * Gets wall heat flux names for connected heat transfers
   */
  std::vector<MaterialPropertyName> getWallHeatFluxNames() const { return _q_wall_names; }

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
   * Get the used closures object(s)
   *
   * @return The closures object(s)
   */
  std::shared_ptr<ClosuresBase> getClosures() const
  {
    mooseDeprecated("getClosures() is deprecated. Use getClosuresObjects() instead.");
    return _closures;
  }
  std::vector<std::shared_ptr<ClosuresBase>> getClosuresObjects() const
  {
    return _closures_objects;
  }

protected:
  virtual std::shared_ptr<FlowModel> buildFlowModel() = 0;
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /**
   * Adds objects which are common for single- and two-phase flow
   */
  virtual void addCommonObjects();

  /**
   * Populates heat connection variable names lists
   */
  virtual void getHeatTransferVariableNames();

  /// The flow model used by this flow channel
  std::shared_ptr<FlowModel> _flow_model;

  /// Name of fluid properties user object
  const UserObjectName & _fp_name;

  /// Angle between orientation vector and gravity vector, in degrees
  const Real _gravity_angle;

  /// Function describing the flow channel area
  FunctionName _area_function;

  /// Closures object(s)
  std::shared_ptr<ClosuresBase> _closures;
  std::vector<std::shared_ptr<ClosuresBase>> _closures_objects;

  const bool & _pipe_pars_transferred;

  /// Roughness of flow channel surface, [m]
  const Real & _roughness;

  /// Convective Heat transfer geometry
  EConvHeatTransGeom _HT_geometry;
  /// Pipe location within the bundle
  EPipeLocation _pipe_location;
  /// Pitch to diameter ratio for parallel bundle heat transfer
  const Real & _PoD;
  /// True if user provides PoD
  bool _has_PoD;

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
  /// wall temperature auxvariable names for connected heat transfers
  std::vector<VariableName> _T_wall_names;
  /// wall temperature material names for connected heat transfers
  std::vector<MaterialPropertyName> _T_wall_mat_names;
  /// wall heat flux names for connected heat transfers
  std::vector<MaterialPropertyName> _q_wall_names;

public:
  static InputParameters validParams();
};

namespace THM
{
template <>
FlowChannelBase::EConvHeatTransGeom stringToEnum(const std::string & s);

template <>
FlowChannelBase::EPipeType stringToEnum(const std::string & s);

template <>
FlowChannelBase::EPipeLocation stringToEnum(const std::string & s);
}
