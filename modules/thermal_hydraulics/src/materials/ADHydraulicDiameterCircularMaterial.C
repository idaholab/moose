#include "ADHydraulicDiameterCircularMaterial.h"

registerMooseObject("ThermalHydraulicsApp", ADHydraulicDiameterCircularMaterial);

InputParameters
ADHydraulicDiameterCircularMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("D_h_name",
                                                "Hydraulic diameter material property name");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  return params;
}

ADHydraulicDiameterCircularMaterial::ADHydraulicDiameterCircularMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _D_h(declareADProperty<Real>(getParam<MaterialPropertyName>("D_h_name"))),
    _area(adCoupledValue("A"))
{
}

void
ADHydraulicDiameterCircularMaterial::computeQpProperties()
{
  _D_h[_qp] = std::sqrt(4. * _area[_qp] / libMesh::pi);
}
