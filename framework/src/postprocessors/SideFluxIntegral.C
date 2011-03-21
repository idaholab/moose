#include "SideFluxIntegral.h"

template<>
InputParameters validParams<SideFluxIntegral>()
{
  InputParameters params = validParams<SidePostprocessor>();
  params.addRequiredParam<std::string>("diffusivity", "The name of the diffusivity material property that will be used in the flux computation.");
  return params;
}

SideFluxIntegral::SideFluxIntegral(const std::string & name, InputParameters parameters) :
    SideIntegral(name, parameters),
    MaterialPropertyInterface(parameters),
    _diffusivity(parameters.get<std::string>("diffusivity")),
    _diffusion_coef(getMaterialProperty<Real>(_diffusivity))
{}

Real
SideFluxIntegral::computeQpIntegral()
{
  return -_diffusion_coef[_qp]*_grad_u[_qp]*_normals[_qp];
}
