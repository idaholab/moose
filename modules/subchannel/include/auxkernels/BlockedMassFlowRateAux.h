#pragma once

#include "AuxKernel.h"
#include "SubChannelMesh.h"

/**
 * Computes mass float rate from specified uniform mass flux and cross-sectional area and applies
 * inlet blockage conditions
 */
class BlockedMassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  BlockedMassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  SubChannelMesh & _subchannel_mesh;
  /// Specified mass flux
  const Real & _mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
  /// index of subchannels affected by blockage
  std::vector<unsigned int> _index_blockage;
};
