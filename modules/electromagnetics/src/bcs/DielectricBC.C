#include "DielectricBC.h"

registerMooseObject("ElectromagneticsApp", DielectricBC);

InputParameters
DielectricBC::validParams()
{
  InputParameters params = DiffusionFluxBC::validParams();
  params.addClassDescription(
      "A first attempt at defining an electric field condition on a dielectric surface, following "
      "Maxwell's Equations and Griffith's 'Introduction to Electrodynamics'");
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
