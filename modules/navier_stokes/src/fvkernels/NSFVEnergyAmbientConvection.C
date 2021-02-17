#include "NSFVEnergyAmbientConvection.h"

registerMooseObject("NavierStokesApp", NSFVEnergyAmbientConvection);

InputParameters
NSFVEnergyAmbientConvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Implements a solid-fluid ambient convection volumetric term "
      "proportional to the difference between the fluid and ambient temperatures : "
      "$q''' = \\alpha (T_{fluid} - T_{ambient})$.");
  params.addRequiredParam<MaterialPropertyName>("alpha",
                                                "Name of the convective heat transfer coefficient");
  params.addRequiredCoupledVar("T_ambient", "Solid ambient temperature");
  return params;
}

NSFVEnergyAmbientConvection::NSFVEnergyAmbientConvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _alpha(getADMaterialProperty<Real>("alpha")),
    _temp_ambient(adCoupledValue("T_ambient"))
{
}

ADReal
NSFVEnergyAmbientConvection::computeQpResidual()
{
  return _alpha[_qp] * (_u[_qp] - _temp_ambient[_qp]);
}
