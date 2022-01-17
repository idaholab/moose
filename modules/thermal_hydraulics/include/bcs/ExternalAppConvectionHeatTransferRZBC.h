#pragma once

#include "ExternalAppConvectionHeatTransferBC.h"
#include "RZSymmetry.h"

/**
 * Convection BC from an external application for RZ domain in XY coordinate system
 */
class ExternalAppConvectionHeatTransferRZBC : public ExternalAppConvectionHeatTransferBC,
                                              public RZSymmetry
{
public:
  ExternalAppConvectionHeatTransferRZBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

public:
  static InputParameters validParams();
};
