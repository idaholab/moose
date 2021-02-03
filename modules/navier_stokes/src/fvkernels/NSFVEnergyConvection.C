#include "NSFVEnergyConvection.h"

registerMooseObject("MooseApp", NSFVEnergyConvection);

InputParameters
NSFVEnergyConvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Implements a solid-fluid convection term proportional "
      "to the difference between the fluid and solid temperatures.");
  params.addRequiredParam<MaterialPropertyName>("h_solid_fluid",
      "Name of the convective heat transfer coefficient");
  params.addRequiredCoupledVar("temp_fluid", "Fluid temperature");
  params.addRequiredCoupledVar("temp_solid", "Solid temperature");
  return params;
}

NSFVEnergyConvection::NSFVEnergyConvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
  _h_solid_fluid(getADMaterialProperty<Real>("h_solid_fluid")),
  _temp_fluid(adCoupledValue("temp_fluid")),
  _temp_solid(adCoupledValue("temp_solid"))
{
}

ADReal
NSFVEnergyConvection::computeQpResidual()
{
  return _h_solid_fluid[_qp] * (_temp_fluid[_qp] - _temp_solid[_qp]);
}
