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
 * Computes mass float rate from total mass flow at the inlet
 */
class SCMFlatMassFlowRateAux : public AuxKernel
{
public:
  static InputParameters validParams();

  SCMFlatMassFlowRateAux(const InputParameters & parameters);

  virtual Real computeValue() override;

protected:
  /// Specified mass flow
  const Real & _mass_flow;
  /// Geometry information
  SubChannelMesh & _subchannel_mesh;
};
