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
MaterialProperty<std::vector<Real> >::init (int size)
{
  MaterialProperty<std::vector<Real> > *copy = new MaterialProperty<std::vector<Real> >;
  copy->_value.resize(size);
  return copy;
}

template <>
PropertyValue *
MaterialProperty<ColumnMajorMatrix>::init (int size)
{
  MaterialProperty<ColumnMajorMatrix> *copy = new MaterialProperty<ColumnMajorMatrix>;
  copy->_value.resize(size);
  for (unsigned int i(0); i < static_cast<unsigned>(size); ++i)
    (*copy)[i].zero();
  return copy;
}

template <>
PropertyValue *
MaterialProperty<std::vector<ColumnMajorMatrix> >::init (int size)
{
  typedef MaterialProperty<std::vector<ColumnMajorMatrix> > PropType;
  PropType *copy = new PropType;
  copy->_value.resize(size);

  // We don't know the size of the underlying vector at each
  // quadrature point, the user will be responsible for resizing it
  // and filling in the entries...

  // Return the copy we allocated
  return copy;
}



template <>
PropertyValue *
MaterialProperty<std::vector<RealTensorValue> >::init (int size)
{
  typedef MaterialProperty<std::vector<RealTensorValue> > PropType;
  PropType *copy = new PropType;
  copy->_value.resize(size);

  // We don't know the size of the underlying vector at each
  // quadrature point, the user will be responsible for resizing it
  // and filling in the entries...

  // Return the copy we allocated
  return copy;
}



template <>
PropertyValue *
MaterialProperty<std::vector<std::vector<RealTensorValue> > >::init (int size)
{
  typedef MaterialProperty<std::vector<std::vector<RealTensorValue> > > PropType;
  PropType *copy = new PropType;
  copy->_value.resize(size);

  // We don't know the size of the underlying vector<vector> at each
  // quadrature point, the user will be responsible for resizing it
  // and filling in the entries...

  // Return the copy we allocated
  return copy;
}
