#include "MaterialProperty.h"

#include "MooseSystem.h"

template <>
PropertyValue *
MaterialProperty<std::vector<Real> >::init ()
{
  MaterialProperty<std::vector<Real> > *copy = new MaterialProperty<std::vector<Real> >;
  libmesh_assert (copy != NULL);

  copy->_value.resize(_value.size());

  return copy;
}
