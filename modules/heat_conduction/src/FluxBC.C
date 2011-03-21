#include "FluxBC.h"

template<>
InputParameters validParams<FluxBC>()
{
  InputParameters params = validParams<NeumannBC>();
  return params;
}

FluxBC::FluxBC(const std::string & name, InputParameters parameters)
  :NeumannBC(name, parameters),
    _k(getMaterialProperty<Real>("thermal_conductivity"))
 {}

Real
FluxBC::computeQpResidual()
  {
    return _k[_qp]*NeumannBC::computeQpResidual();
  }

