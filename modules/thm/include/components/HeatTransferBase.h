#pragma once

#include "ConnectorBase.h"

class HeatTransferBase;
class ClosuresBase;

template <>
InputParameters validParams<HeatTransferBase>();

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
   * Returns wall heat flux name
   *
   * @return The name of wall heat flux variable
   */
  const VariableName & getWallHeatFluxName() const;

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

  /// Used closures
  std::shared_ptr<ClosuresBase> _closures;

  /// heated perimeter name
  VariableName _P_hf_name;
  /// wall temperature name
  VariableName _T_wall_name;
  /// wall heat flux name
  VariableName _q_wall_name;

  /// block IDs corresponding to the connected flow channel
  std::vector<SubdomainID> _block_ids_flow_channel;
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
};
