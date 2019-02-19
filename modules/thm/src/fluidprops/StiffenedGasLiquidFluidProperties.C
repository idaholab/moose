#include "StiffenedGasLiquidFluidProperties.h"
#include <limits>

registerMooseObject("THMApp", StiffenedGasLiquidFluidProperties);

template <>
InputParameters
validParams<StiffenedGasLiquidFluidProperties>()
{
  InputParameters params = validParams<StiffenedGasFluidProperties>();
  params.addClassDescription("Fluid properties for a stiffened gas representing a liquid");
  return params;
}

StiffenedGasLiquidFluidProperties::StiffenedGasLiquidFluidProperties(
    const InputParameters & parameters)
  : StiffenedGasFluidProperties(parameters), LiquidFluidPropertiesInterface()
{
}

Real StiffenedGasLiquidFluidProperties::sigma_from_p_T(Real, Real) const
{
  // return NaN, because surface tension for stiffened gas is not defined
  return std::numeric_limits<Real>::quiet_NaN();
}
