#include "VarCouplingMaterial.h"

template<>
InputParameters validParams<VarCouplingMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("var", "The variable to be coupled in");
  return params;
}


VarCouplingMaterial::VarCouplingMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _var(coupledValue("var")),
    _diffusion(declareProperty<Real>("diffusion"))
{
}

void
VarCouplingMaterial::computeQpProperties()
{
  _diffusion[_qp] = _var[_qp];
}
