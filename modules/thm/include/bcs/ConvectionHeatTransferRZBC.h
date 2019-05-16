#pragma once

#include "ConvectionHeatTransferBC.h"
#include "RZSymmetry.h"

class ConvectionHeatTransferRZBC;

template <>
InputParameters validParams<ConvectionHeatTransferRZBC>();

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
};
