#include "HydraulicDiameterCircularMaterial.h"
#include "FlowModel.h"

registerMooseObject("THMApp", HydraulicDiameterCircularMaterial);

template <>
InputParameters
validParams<HydraulicDiameterCircularMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  return params;
}

HydraulicDiameterCircularMaterial::HydraulicDiameterCircularMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _D_h(declareProperty<Real>(FlowModel::HYDRAULIC_DIAMETER)),
    _area(coupledValue("A"))
{
}

void
HydraulicDiameterCircularMaterial::computeQpProperties()
{
  _D_h[_qp] = std::sqrt(4. * _area[_qp] / libMesh::pi);
}
