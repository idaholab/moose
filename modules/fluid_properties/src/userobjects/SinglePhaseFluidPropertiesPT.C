#include "SinglePhaseFluidPropertiesPT.h"

template<>
InputParameters validParams<SinglePhaseFluidPropertiesPT>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

SinglePhaseFluidPropertiesPT::SinglePhaseFluidPropertiesPT(const InputParameters & parameters) :
    FluidProperties(parameters),
    _R(8.3144621)
{
}

SinglePhaseFluidPropertiesPT::~SinglePhaseFluidPropertiesPT()
{
}

Real
SinglePhaseFluidPropertiesPT::gamma(Real pressure, Real temperature) const
{
  return cp(pressure, temperature) / cv(pressure, temperature);
}
