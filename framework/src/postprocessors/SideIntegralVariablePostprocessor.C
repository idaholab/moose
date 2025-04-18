//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralVariablePostprocessor.h"
#include "MathFVUtils.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideIntegralVariablePostprocessor);

InputParameters
SideIntegralVariablePostprocessor::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable which this postprocessor integrates");
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
    _fv(_fv_variable || _linear_fv_variable)
{
  addMooseVariableDependency(&mooseVariableField());

  _qp_integration = !_fv;
}

Real
SideIntegralVariablePostprocessor::computeFaceInfoIntegral(const FaceInfo * const fi)
{
  return MetaPhysicL::raw_value((*_field_variable)(makeCDFace(*fi), determineState()));
}

Real
SideIntegralVariablePostprocessor::computeQpIntegral()
{
  return _u[_qp];
}

bool
SideIntegralVariablePostprocessor::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  return _field_variable->hasFaceSide(fi, fi_elem_side);
}
