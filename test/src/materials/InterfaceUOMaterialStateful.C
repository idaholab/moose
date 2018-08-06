//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUOMaterialStateful.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", InterfaceUOMaterialStateful);

template <>
InputParameters
validParams<InterfaceUOMaterialStateful>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>(
      "interface_uo_qp", "the name of the ucomputing material property across an interface");
  params.addClassDescription("this material class is used when defining a "
                             "cohesive zone model to store stafeul properties");
  return params;
}

InterfaceUOMaterialStateful::InterfaceUOMaterialStateful(const InputParameters & parameters)
  : Material(parameters),
    _interface_uo_qp(getUserObject<InterfaceUO_QP>("interface_uo_qp")),

    _material_property_average(declareProperty<Real>("material_property_average")),
    _variable_jump(declareProperty<Real>("variable_jump")),
    _boundary_property(declareProperty<Real>("boundary_property")),
    _boundary_property_old(getMaterialPropertyOld<Real>("boundary_property"))

{
}

void
InterfaceUOMaterialStateful::computeQpProperties()
{
  _boundary_property[_qp] =
      _boundary_property_old[_qp] +
      _interface_uo_qp.getNewBoundaryPropertyValue(_current_elem->id(), _current_side, _qp);
  _material_property_average[_qp] =
      _interface_uo_qp.getMeanMatProp(_current_elem->id(), _current_side, _qp);
  _variable_jump[_qp] = _interface_uo_qp.getVarJump(_current_elem->id(), _current_side, _qp);
}

void
InterfaceUOMaterialStateful::initQpStatefulProperties()
{
  _boundary_property[_qp] = 0;
}
