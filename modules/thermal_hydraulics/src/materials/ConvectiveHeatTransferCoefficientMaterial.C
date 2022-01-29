#include "ConvectiveHeatTransferCoefficientMaterial.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ConvectiveHeatTransferCoefficientMaterial);

InputParameters
ConvectiveHeatTransferCoefficientMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("Nu", "Nusselt number");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("k", "Thermal conductivity");
  params.addClassDescription("Computes convective heat transfer coefficient from Nusselt number");
  return params;
}

ConvectiveHeatTransferCoefficientMaterial::ConvectiveHeatTransferCoefficientMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareProperty<Real>("Hw")),
    _Nu(getMaterialProperty<Real>("Nu")),
    _D_h(getMaterialProperty<Real>("D_h")),
    _k(getMaterialProperty<Real>("k"))
{
}

void
ConvectiveHeatTransferCoefficientMaterial::computeQpProperties()
{
  _Hw[_qp] = THM::wallHeatTransferCoefficient(_Nu[_qp], _k[_qp], _D_h[_qp]);
}
