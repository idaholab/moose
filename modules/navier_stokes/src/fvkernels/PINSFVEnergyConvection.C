#include "PINSFVEnergyConvection.h"

registerMooseObject("MooseApp", PINSFVEnergyConvection);

InputParameters
PINSFVEnergyConvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Implements the solid-fluid convection term.");
  params.addRequiredCoupledVar("porosity", "Porosity variable");
  params.addRequiredParam<MaterialPropertyName>("h_solid_fluid", "Name of the convective heat transfer coefficient");
  params.addRequiredParam<bool>("is_solid", "Kernel for the fluid or solid temperature ?");
  params.addRequiredCoupledVar("temp_fluid", "Fluid temperature");
  params.addRequiredCoupledVar("temp_solid", "Solid temperature");
  return params;
}

PINSFVEnergyConvection::PINSFVEnergyConvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
  _eps(coupledValue("porosity")),
  _h_solid_fluid(getADMaterialProperty<Real>("h_solid_fluid")),
  _temp_fluid(adCoupledValue("temp_fluid")),
  _temp_solid(adCoupledValue("temp_solid")),
  _is_solid(getParam<bool>("is_solid"))
{
}

ADReal
PINSFVEnergyConvection::computeQpResidual()
{
  if (_is_solid)
    return -_h_solid_fluid[_qp] * (_temp_fluid[_qp] - _temp_solid[_qp]);
  else
    return _eps[_qp] * _h_solid_fluid[_qp] * (_temp_fluid[_qp] - _temp_solid[_qp]);
}
