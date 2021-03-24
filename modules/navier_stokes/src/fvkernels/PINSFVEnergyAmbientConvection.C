#include "PINSFVEnergyAmbientConvection.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyAmbientConvection);

InputParameters
PINSFVEnergyAmbientConvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Implements the solid-fluid ambient convection term in the porous "
                             "media Navier Stokes energy equation.");
  params.addRequiredParam<MaterialPropertyName>(
      "h_solid_fluid",
      "Name of the convective heat "
      "transfer coefficient. This coefficient should include the influence of porosity.");
  params.addRequiredParam<bool>("is_solid", "Whether this kernel acts on the solid temperature");
  params.addRequiredCoupledVar("temp_fluid", "Fluid temperature");
  params.addRequiredCoupledVar("temp_solid", "Solid temperature");
  return params;
}

PINSFVEnergyAmbientConvection::PINSFVEnergyAmbientConvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _h_solid_fluid(getADMaterialProperty<Real>("h_solid_fluid")),
    _temp_fluid(adCoupledValue("temp_fluid")),
    _temp_solid(adCoupledValue("temp_solid")),
    _is_solid(getParam<bool>("is_solid"))
{
}

ADReal
PINSFVEnergyAmbientConvection::computeQpResidual()
{
  if (_is_solid)
    return -_h_solid_fluid[_qp] * (_temp_fluid[_qp] - _temp_solid[_qp]);
  else
    return _h_solid_fluid[_qp] * (_temp_fluid[_qp] - _temp_solid[_qp]);
}
