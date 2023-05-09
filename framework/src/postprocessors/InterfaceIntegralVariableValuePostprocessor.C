//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceIntegralVariableValuePostprocessor.h"
#include "InterfaceValueTools.h"

registerMooseObject("MooseApp", InterfaceIntegralVariableValuePostprocessor);

InputParameters
InterfaceIntegralVariableValuePostprocessor::validParams()
{
  InputParameters params = InterfaceIntegralPostprocessor::validParams();

  params.addRequiredCoupledVar("variable",
                               "The name of the variable on the primary side of the interface");
  params.addCoupledVar(
      "neighbor_variable",
      "The name of the variable on the secondary side of the interface. By default "
      "the primary side variable name is used for the secondary side as well");
  params.addClassDescription("Add access to variables and their gradient on an interface.");
  params.addParam<MooseEnum>("interface_value_type",
                             InterfaceValueTools::InterfaceAverageOptions(),
                             "Type of value we want to compute");
  return params;
}

InterfaceIntegralVariableValuePostprocessor::InterfaceIntegralVariableValuePostprocessor(
    const InputParameters & parameters)
  : InterfaceIntegralPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _u_neighbor(parameters.isParamSetByUser("neighbor_variable")
                    ? coupledNeighborValue("neighbor_variable")
                    : coupledNeighborValue("variable")),
    _grad_u_neighbor(parameters.isParamSetByUser("neighbor_variable")
                         ? coupledNeighborGradient("neighbor_variable")
                         : coupledNeighborGradient("variable")),
    _interface_value_type(parameters.get<MooseEnum>("interface_value_type")),
    _neighbor_fv_variable(
        parameters.isParamSetByUser("neighbor_variable")
            ? dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("neighbor_variable", 0))
            : dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("variable", 0)))
{
  addMooseVariableDependency(&mooseVariableField());

  // Primary and secondary variable should both be of a similar variable type
  if (parameters.isParamSetByUser("neighbor_variable"))
    if ((_has_fv_vars && !getFieldVar("neighbor_variable", 0)->isFV()) ||
        (!_has_fv_vars && getFieldVar("neighbor_variable", 0)->isFV()))
      mooseError("For the InterfaceIntegralVariableValuePostprocessor, variable and "
                 "neighbor_variable should be of a similar variable type.");
}

Real
InterfaceIntegralVariableValuePostprocessor::computeQpIntegral()
{
  if (_has_fv_vars)
  {
    mooseAssert(_fi, "This should never be null. If it is then something went wrong in execute()");

    // If both variables are different, assume this is a boundary for both variables
    Real u, u_neighbor;
    if (_fv_variable != _neighbor_fv_variable)
    {
      u = MetaPhysicL::raw_value(_fv_variable->getBoundaryFaceValue(*_fi, determineState()));
      u_neighbor = MetaPhysicL::raw_value(
          _neighbor_fv_variable->getBoundaryFaceValue(*_fi, determineState()));
    }
    // If only one variable is specified, assume this is an internal interface
    // FIXME Make sure getInternalFaceValue uses the right interpolation method, see #16585
    else
      u = u_neighbor = MetaPhysicL::raw_value((*_fv_variable)(makeCDFace(*_fi), determineState()));

    return InterfaceValueTools::getQuantity(_interface_value_type, u, u_neighbor);
  }
  else
    return InterfaceValueTools::getQuantity(_interface_value_type, _u[_qp], _u_neighbor[_qp]);
}

bool
InterfaceIntegralVariableValuePostprocessor::hasFaceSide(const FaceInfo &, bool) const
{
  // Our default interface kernel treats elem and neighbor sides equivalently so we will assume for
  // now that we will happily consume functor evaluations on either side of a face and any
  // interpolation between said evaluations
  return true;
}
