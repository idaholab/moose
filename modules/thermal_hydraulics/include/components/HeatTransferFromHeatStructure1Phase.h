#pragma once

#include "HeatTransferFromTemperature1Phase.h"
#include "HSBoundaryInterface.h"
#include "FlowChannelAlignment.h"

/**
 * Connects a 1-phase flow channel and a heat structure
 */
class HeatTransferFromHeatStructure1Phase : public HeatTransferFromTemperature1Phase,
                                            public HSBoundaryInterface
{
public:
  HeatTransferFromHeatStructure1Phase(const InputParameters & parameters);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual const FEType & getFEType() override;

  virtual void preSetupMesh() override;
  virtual void setupMesh() override;
  virtual void check() const override;

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

  FlowChannelAlignment _fch_alignment;

public:
  static InputParameters validParams();
};
