//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConnectorBase.h"

class ClosuresBase;

/**
 * Base class for heat transfer connections
 */
class HeatTransferBase : public ConnectorBase
{
public:
  HeatTransferBase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  /**
   * Returns heated perimeter name
   *
   * @return The name of heated perimeter variable
   */
  const VariableName & getHeatedPerimeterName() const;
  /**
   * Returns wall temperature name
   *
   * @return The name of wall temperature variable
   */
  const VariableName & getWallTemperatureName() const;

  /**
   * Returns wall temperature name
   *
   * @return The name of wall temperature material
   */
  const MaterialPropertyName & getWallTemperatureMatName() const;

  /**
   * Returns wall heat flux name
   *
   * @return The name of wall heat flux material property
   */
  const MaterialPropertyName & getWallHeatFluxName() const;

  /**
   * Returns whether this heat transfer is specified by temperature, rather than heat flux
   *
   * @return true if the heat transfer is specified by temperature, false otherwise
   */
  virtual bool isTemperatureType() const = 0;

  /**
   * Get the list of the subdomain names associated with the flow channel
   *
   * @return List of subdomain names associated with the flow channel
   */
  const std::vector<SubdomainName> & getFlowChannelSubdomains() const
  {
    return _flow_channel_subdomains;
  }

  const UserObjectName & getFluidPropertiesName() const;

  /**
   * Get the name of the connected flow channel
   */
  const std::string & getFlowChannelName() const { return _flow_channel_name; }

protected:
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /**
   * Adds heated perimeter variable and objects
   */
  void addHeatedPerimeter();

  /// name of the connected flow channel
  const std::string _flow_channel_name;

  /// flag that heated perimeter is transferred from another application
  const bool _P_hf_transferred;
  /// flag that the heated perimeter was specified via an input parameter
  const bool _P_hf_provided;

  /// Used closures object(s)
  std::shared_ptr<ClosuresBase> _closures;
  std::vector<std::shared_ptr<ClosuresBase>> _closures_objects;

  /// heated perimeter name
  VariableName _P_hf_name;
  /// wall temperature name
  VariableName _T_wall_name;
  /// wall temperature material name
  MaterialPropertyName _T_wall_mat_name;
  /// wall heat flux name
  MaterialPropertyName _q_wall_name;

  /// Subdomains corresponding to the connected flow channel
  std::vector<SubdomainName> _flow_channel_subdomains;
  /// flow model type
  THM::FlowModelID _model_type;
  /// fluid properties object name
  UserObjectName _fp_name;
  /// area function name for the connected flow channel
  FunctionName _A_fn_name;
  /// heated perimeter function name
  FunctionName _P_hf_fn_name;

public:
  static InputParameters validParams();
};
