#include "GenericDiffusion.h"

template<>
InputParameters validParams<GenericDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("property", "The name of the material property to use as the diffusivity.");
  return params;
}

GenericDiffusion::GenericDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _diffusivity(getMaterialProperty<Real>(parameters.get<std::string>("property")))
{
}

Real
GenericDiffusion::computeQpResidual()
{
  return _diffusivity[_qp]*_grad_test[_i][_qp]*_grad_u[_qp];
}

Real
GenericDiffusion::computeQpJacobian()
{
  return _diffusivity[_qp]*_grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
