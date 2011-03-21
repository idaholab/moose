/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MaterialPropertyInterface.h"
#include "SubProblem.h"
#include "MaterialData.h"

MaterialPropertyInterface::MaterialPropertyInterface(InputParameters & parameters) :
    _material_data(*parameters.get<MaterialData *>("_material_data")),
    _material_props(_material_data.props()),
    _material_props_old(_material_data.propsOld()),
    _material_props_older(_material_data.propsOlder())
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

template <>
PropertyValue *
MaterialProperty<ColumnMajorMatrix>::init ()
{
  MaterialProperty<ColumnMajorMatrix> *copy = new MaterialProperty<ColumnMajorMatrix>;
  libmesh_assert (copy != NULL);

  copy->_value.resize(_value.size());

  for (unsigned int i(0); i < _value.size(); ++i)
  {
    (*copy)[i].zero();
  }

  return copy;
}
