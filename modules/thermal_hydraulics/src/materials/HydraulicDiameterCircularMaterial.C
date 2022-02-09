#include "HydraulicDiameterCircularMaterial.h"

registerMooseObject("ThermalHydraulicsApp", HydraulicDiameterCircularMaterial);

InputParameters
HydraulicDiameterCircularMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("D_h_name",
                                                "Hydraulic diameter material property name");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  return params;
}

HydraulicDiameterCircularMaterial::HydraulicDiameterCircularMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _D_h(declareProperty<Real>(getParam<MaterialPropertyName>("D_h_name"))),
    _area(coupledValue("A"))
{
}

void
HydraulicDiameterCircularMaterial::computeQpProperties()
{
  _D_h[_qp] = std::sqrt(4. * _area[_qp] / libMesh::pi);
}
