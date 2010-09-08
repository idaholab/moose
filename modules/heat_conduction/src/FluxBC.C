#include "FluxBC.h"

template<>
InputParameters validParams<FluxBC>()
{
  InputParameters params = validParams<NeumannBC>();
  return params;
}

FluxBC::FluxBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :NeumannBC(name, moose_system, parameters),
    _k(getMaterialProperty<Real>("thermal_conductivity"))
 {}

Real
FluxBC::computeQpResidual()
  {
    return _k[_qp]*NeumannBC::computeQpResidual();
  }

