/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMMaterialManagerConstraint.h"

#include "XFEMElemPairMaterialManager.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<XFEMMaterialManagerConstraint>()
{
  InputParameters params = validParams<ElemElemConstraint>();
  params.addRequiredParam<UserObjectName>("manager", "XFEMElemPairMaterialManager object");
  return params;
}

XFEMMaterialManagerConstraint::XFEMMaterialManagerConstraint(const InputParameters & parameters)
  : ElemElemConstraint(parameters), _manager(getUserObject<XFEMElemPairMaterialManager>("manager"))
{
}

void
XFEMMaterialManagerConstraint::computeResidual()
{
  _manager.swapInProperties(_current_elem->id() + _neighbor_elem->id());
  ElemElemConstraint::computeResidual();
  _manager.swapOutProperties(_current_elem->id() + _neighbor_elem->id());
}

void
XFEMMaterialManagerConstraint::computeJacobian()
{
  _manager.swapInProperties(_current_elem->id() + _neighbor_elem->id());
  ElemElemConstraint::computeJacobian();
  _manager.swapOutProperties(_current_elem->id() + _neighbor_elem->id());
}
