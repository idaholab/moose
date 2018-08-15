#include "NumericalFlux3EqnBase.h"

template <>
InputParameters
validParams<NumericalFlux3EqnBase>()
{
  InputParameters params = validParams<RDGFluxBase>();
  return params;
}

NumericalFlux3EqnBase::NumericalFlux3EqnBase(const InputParameters & parameters)
  : RDGFluxBase(parameters),
    RDGIndices3Eqn(),

    _last_region_index(0)
{
}
