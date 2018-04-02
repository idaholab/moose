#include "DirectionMaterial.h"

registerMooseObject("RELAP7App", DirectionMaterial);

template <>
InputParameters
validParams<DirectionMaterial>()
{
  InputParameters params = validParams<Material>();
  params.set<MooseEnum>("constant_on") = 1; // constant on element
  return params;
}

DirectionMaterial::DirectionMaterial(const InputParameters & parameters)
  : Material(parameters), _dir(declareProperty<RealVectorValue>("direction"))
{
}

void
DirectionMaterial::computeQpProperties()
{
  const Elem * el = _mesh.elemPtr(_current_elem->id());
  RealVectorValue dir = el->node_ref(1) - el->node_ref(0);
  _dir[_qp] = dir / dir.norm();
}
