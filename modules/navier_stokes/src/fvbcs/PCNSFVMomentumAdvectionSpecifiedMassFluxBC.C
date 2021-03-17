#include "PCNSFVMomentumAdvectionSpecifiedMassFluxBC.h"
#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", PCNSFVMomentumAdvectionSpecifiedMassFluxBC);

InputParameters
PCNSFVMomentumAdvectionSpecifiedMassFluxBC::validParams()
{
  auto params = PNSFVMassSpecifiedMassFluxBC::validParams();
  params.addClassDescription("Momentum advection BC for specified mass flux.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

PCNSFVMomentumAdvectionSpecifiedMassFluxBC::PCNSFVMomentumAdvectionSpecifiedMassFluxBC(
    const InputParameters & parameters)
  : PNSFVMassSpecifiedMassFluxBC(parameters),
    _rho(getADMaterialProperty<Real>(NS::density)),
    _eps(getMaterialProperty<Real>(NS::porosity)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

void
PCNSFVMomentumAdvectionSpecifiedMassFluxBC::computeMemberData()
{
  PNSFVMassSpecifiedMassFluxBC::computeMemberData();

  _velocity.assign(ADRealVectorValue(_mass_flux(0) / _rho[_qp] / _eps[_qp], 0, 0));
  if (_superficial_rhov)
    _velocity(1) = _mass_flux(1) / _rho[_qp] / _eps[_qp];
  if (_superficial_rhow)
    _velocity(2) = _mass_flux(2) / _rho[_qp] / _eps[_qp];
}

ADReal
PCNSFVMomentumAdvectionSpecifiedMassFluxBC::computeQpResidual()
{
  mooseAssert(this->hasBlocks(_face_info->elem().subdomain_id()), "Checking block restriction");

  computeMemberData();

  return _normal * _mass_flux * _velocity(_index);
}
