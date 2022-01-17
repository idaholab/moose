#pragma once

#include "ADExternalAppConvectionHeatTransferBC.h"
#include "RZSymmetry.h"

/**
 * Convection BC from an external application for RZ domain in XY coordinate system
 */
class ADExternalAppConvectionHeatTransferRZBC : public ADExternalAppConvectionHeatTransferBC,
                                                public RZSymmetry
{
public:
  ADExternalAppConvectionHeatTransferRZBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

public:
  static InputParameters validParams();
};
