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

#include "MaterialPropertyUserObject.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<MaterialPropertyUserObject>()
{
  InputParameters params = validParams<ElementIntegralUserObject>();
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
