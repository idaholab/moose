#include "MaterialPropertyInterface.h"
#include "SubProblem.h"
#include "MaterialData.h"

namespace Moose {

MaterialPropertyInterface::MaterialPropertyInterface(InputParameters & parameters) :
    _material_data(*parameters.get<Moose::MaterialData *>("_material_data")),
    _material_props(_material_data.props()),
    _material_props_old(_material_data.propsOld()),
    _material_props_older(_material_data.propsOlder())
{
}

} // namespace

template <>
Moose::PropertyValue *
MaterialProperty<std::vector<Real> >::init ()
{
  MaterialProperty<std::vector<Real> > *copy = new MaterialProperty<std::vector<Real> >;
  libmesh_assert (copy != NULL);

  copy->_value.resize(_value.size());

  return copy;
}

