#include "MaterialPropertyInterface.h"
#include "SubProblem.h"

namespace Moose {

MaterialPropertyInterface::MaterialPropertyInterface(InputParameters & parameters) :
    _subproblem(*parameters.get<Moose::SubProblem *>("_problem")),
    _material_props(_subproblem.materialProps()),
    _material_props_old(_subproblem.materialPropsOld()),
    _material_props_older(_subproblem.materialPropsOlder())
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

