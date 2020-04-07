#pragma once

#include "ConvectionHeatTransferBC.h"
#include "RZSymmetry.h"

/**
 * Convection BC for RZ domain in XY coordinate system
 */
class ConvectionHeatTransferRZBC : public ConvectionHeatTransferBC, public RZSymmetry
{
public:
  ConvectionHeatTransferRZBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

public:
  static InputParameters validParams();
};
