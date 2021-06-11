#include "ADDirectionMaterial.h"

registerMooseObject("THMApp", ADDirectionMaterial);

InputParameters
ADDirectionMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.set<MooseEnum>("constant_on") = 1; // constant on element
  return params;
}

ADDirectionMaterial::ADDirectionMaterial(const InputParameters & parameters)
  : Material(parameters), _dir(declareADProperty<RealVectorValue>("direction"))
{
}

void
ADDirectionMaterial::computeQpProperties()
{
  const Elem * el = _mesh.elemPtr(_current_elem->id());
  RealVectorValue dir = el->node_ref(1) - el->node_ref(0);
  _dir[_qp] = dir / dir.norm();
}
