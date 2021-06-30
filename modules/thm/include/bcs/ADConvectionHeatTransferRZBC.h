#pragma once

#include "ADConvectionHeatTransferBC.h"
#include "RZSymmetry.h"

/**
 * Convection BC for RZ domain in XY coordinate system
 */
class ADConvectionHeatTransferRZBC : public ADConvectionHeatTransferBC, public RZSymmetry
{
public:
  ADConvectionHeatTransferRZBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

public:
  static InputParameters validParams();
};
