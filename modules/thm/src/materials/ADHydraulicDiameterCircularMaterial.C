#include "ADHydraulicDiameterCircularMaterial.h"
#include "FlowModel.h"

registerMooseObject("THMApp", ADHydraulicDiameterCircularMaterial);

InputParameters
ADHydraulicDiameterCircularMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  return params;
}

ADHydraulicDiameterCircularMaterial::ADHydraulicDiameterCircularMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _D_h(declareADProperty<Real>(FlowModel::HYDRAULIC_DIAMETER)),
    _area(adCoupledValue("A"))
{
}

void
ADHydraulicDiameterCircularMaterial::computeQpProperties()
{
  _D_h[_qp] = std::sqrt(4. * _area[_qp] / libMesh::pi);
}
