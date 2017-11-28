/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StatefulMaterialJump.h"

template <>
InputParameters
validParams<StatefulMaterialJump>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("");
  params.addRequiredCoupledVar("u", "Name of the variable to couple");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same block");
  return params;
}

StatefulMaterialJump::StatefulMaterialJump(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _prop(declareProperty<Real>(_base_name + "jump")),
    _prop_old(getMaterialPropertyOld<Real>(_base_name + "jump")),
    _u(coupledValue("u")),
    _u_neighbor(coupledNeighborValue("u"))
{
}

void
StatefulMaterialJump::resetQpProperties()
{
  _prop[_qp] = 0.0;
}

void
StatefulMaterialJump::computeQpProperties()
{
  // Real old_value = _prop_old[_qp];
  //_prop[_qp] = _u[_qp] - _u_neighbor[_qp];
  // std::cout << "prop[" << _qp << "] = " << _prop[_qp] << std::endl;
  _prop[_qp] = _prop_old[_qp] + 0.1 * _t;
}

void
StatefulMaterialJump::initQpStatefulProperties()
{
  _prop[_qp] = 0.0;
}
