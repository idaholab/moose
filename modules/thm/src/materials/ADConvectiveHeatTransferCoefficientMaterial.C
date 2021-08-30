#include "ADConvectiveHeatTransferCoefficientMaterial.h"
#include "Numerics.h"

registerMooseObject("THMApp", ADConvectiveHeatTransferCoefficientMaterial);

InputParameters
ADConvectiveHeatTransferCoefficientMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("Nu", "Nusselt number");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredParam<MaterialPropertyName>("k", "Thermal conductivity");
  params.addClassDescription("Computes convective heat transfer coefficient from Nusselt number");
  return params;
}

ADConvectiveHeatTransferCoefficientMaterial::ADConvectiveHeatTransferCoefficientMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _Hw(declareADProperty<Real>("Hw")),
    _Nu(getADMaterialProperty<Real>("Nu")),
    _D_h(getMaterialProperty<Real>("D_h")),
    _k(getADMaterialProperty<Real>("k"))
{
}

void
ADConvectiveHeatTransferCoefficientMaterial::computeQpProperties()
{
  _Hw[_qp] = THM::wallHeatTransferCoefficient(_Nu[_qp], _k[_qp], _D_h[_qp]);
}
