#ifndef HEATTRANSFERBASE_H
#define HEATTRANSFERBASE_H

#include "ConnectorBase.h"

class HeatTransferBase;

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

protected:
  virtual void init() override;
  virtual void initSecondary() override;
  virtual void check() const override;

  /**
   * Adds heated perimeter variable and objects
   */
  void addHeatedPerimeter();

  /// name of the connected pipe
  const std::string _pipe_name;

  /// flag that heated perimeter is transferred from another application
  const bool _P_hf_transferred;
  /// flag that the heated perimeter was specified via an input parameter
  const bool _P_hf_provided;

  /// name of the chosen closures
  std::string _closures_name;

  /// heated perimeter name
  VariableName _P_hf_name;
  /// wall temperature name
  VariableName _T_wall_name;
  /// wall heat flux name
  VariableName _q_wall_name;

  /// block IDs corresponding to the connected pipe
  std::vector<unsigned int> _block_ids_pipe;
  /// flow model type
  THM::FlowModelID _model_type;
  /// fluid properties object name
  UserObjectName _fp_name;
  /// area function name for the connected pipe
  FunctionName _A_fn_name;
  /// heated perimeter function name
  FunctionName _P_hf_fn_name;
};

#endif /* HEATTRANSFERBASE_H */
