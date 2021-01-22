#include "PINSFVEnergyConvection.h"

registerMooseObject("MooseApp", PINSFVEnergyConvection);

InputParameters
PINSFVEnergyConvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Implements the solid-fluid convection term.");
  params.addRequiredParam<MaterialPropertyName>("h_fs", "Name of the convective heat transfer coefficient");
  params.addRequiredParam<bool>("is_solid", "Kernel for the fluid or solid temperature ?");
  params.addRequiredCoupledVar("temp_fluid", "Fluid temperature");
  params.addRequiredCoupledVar("temp_solid", "Solid temperature");
  return params;
}

PINSFVEnergyConvection::PINSFVEnergyConvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
  _h_solid_fluid(getADMaterialProperty<Real>("h_solid_fluid")),
  _temp_fluid(adCoupledValue("temp_fluid")),
  _temp_solid(adCoupledValue("temp_solid")),
  _is_solid(getParam<bool>("is_solid"))
{
}

ADReal
PINSFVEnergyConvection::computeQpResidual()
{
  return (2 * _is_solid - 1) * _h_solid_fluid[_qp] * (_temp_fluid[_qp] - _temp_solid[_qp]);
}
