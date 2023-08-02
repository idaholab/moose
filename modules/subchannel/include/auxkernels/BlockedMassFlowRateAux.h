/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

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
  const Real & _unblocked_mass_flux;
  /// Specified mass flow rate
  const Real & _blocked_mass_flux;
  /// Cross-sectional area
  const VariableValue & _area;
  /// index of subchannels affected by blockage
  std::vector<unsigned int> _index_blockage;
};
