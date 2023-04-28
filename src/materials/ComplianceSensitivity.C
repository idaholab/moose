#include "ComplianceSensitivity.h"

registerMooseObject("troutApp", ComplianceSensitivity);

InputParameters
ComplianceSensitivity::validParams()
{
  InputParameters params = StrainEnergyDensity::validParams();
  params.addClassDescription("Computes compliance sensitivity.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredCoupledVar("filtered_design_density", "Filtered design density variable name.");
  params.addRequiredParam<int>("power", "Penalty power for SIMP method.");
  params.addRequiredParam<Real>("E", "Young's modulus for the material.");
  params.addRequiredParam<Real>("Emin", "Minimum value of Young's modulus for the material.");
  // Add any new parameters here, if required

  return params;
}

ComplianceSensitivity::ComplianceSensitivity(const InputParameters & parameters)
  : StrainEnergyDensity(parameters),
    _sensitivity(declareProperty<Real>(_base_name + "sensitivity")),
    _filtered_design_density(coupledValue("filtered_design_density")),
    _design_density(coupledValue("design_density")),
    _power(getParam<int>("power")),
    _E(getParam<Real>("E")),
    _Emin(getParam<Real>("Emin"))
{
}

void
ComplianceSensitivity::computeQpProperties()
{
  StrainEnergyDensity::computeQpProperties();
  // ce = reshape(sum((U(edofMat)*KE).*U(edofMat),2),nely,nelx);
  Real ce = _power * _strain_energy_density[_qp];
  // dc = -penal*(E0-Emin)*xPhys.^(penal-1).*ce;
  Real derivative =
      -MathUtils::pow((_power * (_E - _Emin) * _filtered_design_density[_qp]), _power - 1) * ce;

  // dc(:) = H*(x(:).*dc(:))./Hs./max(1e-3,x(:));
  // multiplying by _design_density now for the radial average filter
  _sensitivity[_qp] = derivative * _design_density[_qp];
}
