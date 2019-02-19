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
   * Returns 1-phase wall heat transfer coefficient name
   *
   * @return The name of the 1-phase wall heat transfer coefficient variable
   */
  const MaterialPropertyName & getWallHeatTransferCoefficient1PhaseName() const;
  /**
   * Returns liquid wall heat transfer coefficient name
   *
   * @return The name of liquid wall heat transfer coefficient variable
   */
  const MaterialPropertyName & getWallHeatTransferCoefficientLiquidName() const;
  /**
   * Returns vapor wall heat transfer coefficient name
   *
   * @return The name of vapor wall heat transfer coefficient variable
   */
  const MaterialPropertyName & getWallHeatTransferCoefficientVaporName() const;
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
  /**
   * Adds objects common to 1-phase and 2-phase
   */
  void addMooseObjectsCommon();
  /**
   * Adds 1-phase objects
   */
  void addMooseObjects1Phase();
  /**
   * Adds 2-phase objects
   */
  void addMooseObjects2Phase();

  /// name of the connected pipe
  const std::string _pipe_name;

  /// flag that heated perimeter is transferred from another application
  const bool _P_hf_transferred;
  /// flag that the heated perimeter was specified via an input parameter
  const bool _P_hf_provided;

  /// name of the chosen closures
  std::string _closures_name;

  /// 1-phase wall heat transfer coefficient name
  MaterialPropertyName _Hw_1phase_name;
  /// liquid wall heat transfer coefficient name
  MaterialPropertyName _Hw_liquid_name;
  /// vapor wall heat transfer coefficient name
  MaterialPropertyName _Hw_vapor_name;
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
  /// flag signalling that there is phase interaction (relevant only in 2-phase)
  bool _phase_interaction;
  /// fluid properties object name
  UserObjectName _fp_name;
  /// area function name for the connected pipe
  FunctionName _A_fn_name;
  /// heated perimeter function name
  FunctionName _P_hf_fn_name;
};

#endif /* HEATTRANSFERBASE_H */
