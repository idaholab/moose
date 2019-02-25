#ifndef FLOWCHANNEL_H
#define FLOWCHANNEL_H

#include "GeometricalFlowComponent.h"

class FlowChannel;

template <>
InputParameters validParams<FlowChannel>();

/**
 * A class representing a flow channel
 *
 * A flow channel is defined by its position, direction, length and area.
 */
class FlowChannel : public GeometricalFlowComponent
{
public:
  FlowChannel(const InputParameters & params);

  /// Type of convective heat transfer geometry
  enum EConvHeatTransGeom
  {
    PIPE,      ///< pipe geometry
    ROD_BUNDLE ///< rod bundle geometry
  };
  /// Map of convective heat transfer geometry type to enum
  static const std::map<std::string, EConvHeatTransGeom> _heat_transfer_geom_to_enum;

  /**
   * Gets a MooseEnum for convective heat transfer geometry type
   *
   * @param[in] name   default value
   * @returns MooseEnum for convective heat transfer geometry type
   */
  static MooseEnum getConvHeatTransGeometry(const std::string & name);

  // Pipe specific interface ----
  virtual std::shared_ptr<const FlowModel> getFlowModel() const;
  virtual unsigned int getSubdomainID() const = 0;
  virtual bool isHorizontal() const = 0;

  /**
   * Get the name of the function describing the flow channel area
   *
   * @return The name of the function describing the flow channel area
   */
  const FunctionName & getAreaFunctionName() const { return _area_function; }

protected:
  virtual std::shared_ptr<FlowModel> buildFlowModel();
  virtual void init() override;

  /// The flow model used by this pipe
  std::shared_ptr<FlowModel> _flow_model;

  /// Function describing the flow channel area
  const FunctionName & _area_function;
};

#endif /* FLOWCHANNEL_H */
