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
      "the same variable name is used for the secondary side");
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
            : dynamic_cast<const MooseVariableFV<Real> *>(getFieldVar("variable", 0))),
    _fv(_fv_variable)
{
  addMooseVariableDependency(&mooseVariableField());

  // Primary and secondary variable should both be of a similar variable type
  if (parameters.isParamSetByUser("neighbor_variable"))
    if ((_fv && !getFieldVar("neighbor_variable", 0)->isFV()) ||
        (!_fv && getFieldVar("neighbor_variable", 0)->isFV()))
      mooseError("For the InterfaceIntegralVariableValuePostprocessor, variable and "
                 "neighbor_variable should be of a similar variable type.");
}

Real
InterfaceIntegralVariableValuePostprocessor::computeQpIntegral()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_fv)
  {
    // Get FaceInfo from the mesh
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");
    const Elem * const neighbor = _current_elem->neighbor_ptr(_current_side);

    // If both variables are different, assume this is a boundary for both variables
    Real u, u_neighbor;
    if (_fv_variable != _neighbor_fv_variable)
    {
      u = MetaPhysicL::raw_value(_fv_variable->getBoundaryFaceValue(*fi));
      u_neighbor = MetaPhysicL::raw_value(_neighbor_fv_variable->getBoundaryFaceValue(*fi));
    }
    // If only one variable is specified, assume this is an internal interface
    // FIXME Make sure getInternalFaceValue uses the right interpolation method, see #16585
    else
    {
      u = MetaPhysicL::raw_value(_fv_variable->getInternalFaceValue(neighbor, *fi, _u[_qp]));
      u_neighbor = MetaPhysicL::raw_value(
          _neighbor_fv_variable->getInternalFaceValue(neighbor, *fi, _u_neighbor[_qp]));
    }

    return InterfaceValueTools::getQuantity(_interface_value_type, u, u_neighbor);
  }
  else
#endif
    return InterfaceValueTools::getQuantity(_interface_value_type, _u[_qp], _u_neighbor[_qp]);
}
