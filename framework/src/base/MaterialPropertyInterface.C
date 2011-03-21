#include "MaterialPropertyInterface.h"
#include "SubProblem.h"

namespace Moose {

MaterialPropertyInterface::MaterialPropertyInterface(SubProblem &problem) :
  _material_props(problem.materialProps()),
  _material_props_old(problem.materialPropsOld()),
  _material_props_older(problem.materialPropsOlder())
{
}

template <>
PropertyValue *
MaterialProperty<std::vector<Real> >::init ()
{
  MaterialProperty<std::vector<Real> > *copy = new MaterialProperty<std::vector<Real> >;
  libmesh_assert (copy != NULL);

  copy->_value.resize(_value.size());

  return copy;
}

} // namespace
