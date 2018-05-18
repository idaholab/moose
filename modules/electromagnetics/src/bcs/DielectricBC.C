#include "DielectricBC.h"

registerMooseObject("ElkApp", DielectricBC);

template <>
InputParameters
validParams<DielectricBC>()
{
  InputParameters params = validParams<DiffusionFluxBC>();
  params.addParam<Real>("epsR_one", 1.0, "Relative Permittivity for computational region");
  params.addParam<Real>("epsR_two", 1.0, "Relative Permittivity for region beyond boundary");
  return params;
}

DielectricBC::DielectricBC(const InputParameters & parameters)
  : DiffusionFluxBC(parameters),

    _eps_one(getParam<Real>("epsR_one")),
    _eps_two(getParam<Real>("epsR_two"))
{
}

Real
DielectricBC::computeQpResidual()
{
  return (_eps_two / _eps_one) * DiffusionFluxBC::computeQpResidual();
}

Real
DielectricBC::computeQpJacobian()
{
  return (_eps_two / _eps_one) * DiffusionFluxBC::computeQpJacobian();
}
