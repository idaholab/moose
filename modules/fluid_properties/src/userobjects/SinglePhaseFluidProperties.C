#include "SinglePhaseFluidProperties.h"

template<>
InputParameters validParams<SinglePhaseFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

SinglePhaseFluidProperties::SinglePhaseFluidProperties(const InputParameters & parameters) :
    FluidProperties(parameters)
{
}

SinglePhaseFluidProperties::~SinglePhaseFluidProperties()
{
}

Real
SinglePhaseFluidProperties::gamma(Real v, Real u) const
{
  return cp(v, u) / cv(v, u);
}
