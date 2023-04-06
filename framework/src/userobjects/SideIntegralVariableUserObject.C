//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralVariableUserObject.h"

InputParameters
SideIntegralVariableUserObject::validParams()
{
  InputParameters params = SideIntegralUserObject::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this boundary condition applies to");
  return params;
}

SideIntegralVariableUserObject::SideIntegralVariableUserObject(const InputParameters & parameters)
  : SideIntegralUserObject(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _fv(_fv_variable)
{
  addMooseVariableDependency(&mooseVariableField());
}

Real
SideIntegralVariableUserObject::computeQpIntegral()
{
  if (_fv)
  {
    // We should be at the edge of the domain for this variable
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");

    return MetaPhysicL::raw_value(_fv_variable->getBoundaryFaceValue(*fi, determineState()));
  }
  else
    return _u[_qp];
}
