#pragma once

#include "HeatTransferFromTemperature1Phase.h"

class HeatTransferFromHeatStructure1Phase;

template <>
InputParameters validParams<HeatTransferFromHeatStructure1Phase>();

/**
 * Connects a 1-phase flow channel and a heat structure
 */
class HeatTransferFromHeatStructure1Phase : public HeatTransferFromTemperature1Phase
{
public:
  HeatTransferFromHeatStructure1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;
  void checkFlowChannelAlignment() const;

  /**
   * Gets the "master side" (heat structure) boundary name for this connection
   *
   * @return The boundary name of the master side
   */
  const BoundaryName & getMasterSideName() const;
  /**
   * Gets the "slave side" (flow channel) nodeset name for this connection
   *
   * @return The nodeset name for the slave side
   */
  const BoundaryName & getSlaveSideName() const;

  /// heat structure name
  const std::string _hs_name;
  /// "side" of the heat structure
  const MooseEnum _hs_side;
};
