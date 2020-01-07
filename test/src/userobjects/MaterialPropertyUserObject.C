//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialPropertyUserObject.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", MaterialPropertyUserObject);

InputParameters
MaterialPropertyUserObject::validParams()
{
  InputParameters params = ElementIntegralUserObject::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "mat_prop", "the name of the material property we are going to use");
  return params;
}

MaterialPropertyUserObject::MaterialPropertyUserObject(const InputParameters & parameters)
  : ElementIntegralUserObject(parameters), _mat_prop(getMaterialProperty<Real>("mat_prop"))
{
}

void
MaterialPropertyUserObject::initialize()
{
  ElementIntegralUserObject::initialize();

  _elem_integrals.clear();
  _elem_integrals.resize(_subproblem.mesh().getMesh().max_elem_id());
}

void
MaterialPropertyUserObject::execute()
{
  Real integral_value = computeIntegral();

  _elem_integrals[_current_elem->id()] = integral_value;
}

void
MaterialPropertyUserObject::finalize()
{
  gatherSum(_elem_integrals);
}

void
MaterialPropertyUserObject::threadJoin(const UserObject & y)
{
  ElementIntegralUserObject::threadJoin(y);
  const MaterialPropertyUserObject & mat_uo = dynamic_cast<const MaterialPropertyUserObject &>(y);
  for (unsigned int i = 0; i < _elem_integrals.size(); i++)
    _elem_integrals[i] += mat_uo._elem_integrals[i];
}

Real
MaterialPropertyUserObject::computeQpIntegral()
{
  return _mat_prop[_qp];
}

Real
MaterialPropertyUserObject::getElementalValue(unsigned int elem_id) const
{
  if (_elem_integrals.size() > 0)
    return _elem_integrals[elem_id];
  else
    return 0.;
}
