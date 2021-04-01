#pragma once

#include "AuxKernel.h"
#include "SubChannelMeshBase.h"

/**
 * Computes mass float rate from total mass flow at the inlet
 */
class UniformlyDistributedMassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  UniformlyDistributedMassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// Specified mass flow
  const Real & _mass_flow;
  /// Geometry information
  SubChannelMeshBase & _subchannel_mesh;
};
