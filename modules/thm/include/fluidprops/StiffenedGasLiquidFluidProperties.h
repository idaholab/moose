#ifndef STIFFENEDGASLIQUIDFLUIDPROPERTIES_H
#define STIFFENEDGASLIQUIDFLUIDPROPERTIES_H

#include "StiffenedGasFluidProperties.h"
#include "LiquidFluidPropertiesInterface.h"

class StiffenedGasLiquidFluidProperties;

template <>
InputParameters validParams<StiffenedGasLiquidFluidProperties>();

/**
 * Stiffened gas fluid properties representing liquid
 */
class StiffenedGasLiquidFluidProperties : public StiffenedGasFluidProperties,
                                          public LiquidFluidPropertiesInterface
{
public:
  StiffenedGasLiquidFluidProperties(const InputParameters & parameters);

  virtual Real sigma_from_p_T(Real p, Real T) const override;
};

#endif /* STIFFENEDGASLIQUIDFLUIDPROPERTIES_H */
