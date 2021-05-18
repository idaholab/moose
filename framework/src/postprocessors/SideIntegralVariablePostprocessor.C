//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralVariablePostprocessor.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideIntegralVariablePostprocessor);

defineLegacyParams(SideIntegralVariablePostprocessor);

InputParameters
SideIntegralVariablePostprocessor::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this boundary condition applies to");
  params.addClassDescription("Computes a surface integral of the specified variable");
  return params;
}

SideIntegralVariablePostprocessor::SideIntegralVariablePostprocessor(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
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
SideIntegralVariablePostprocessor::computeQpIntegral()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_fv)
  {
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have an fi");
    return MetaPhysicL::raw_value(_fv_variable->getBoundaryFaceValue(*fi));
  }
  else
#endif
    return _u[_qp];
}
