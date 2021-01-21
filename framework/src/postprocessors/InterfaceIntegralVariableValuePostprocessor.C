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
    _fv(_fv_variable)
{
  addMooseVariableDependency(&mooseVariableField());

  /// Check that the secondary variable is also a finite volume variable
  if (_fv)
    if (!getFieldVar("neighbor_variable", 0)->isFV())
      mooseError("For the InterfaceIntegralVariableValuePostprocessor, if variable "
                 "is a finite volume variable, so should be neighbor_variable.");
}

Real
InterfaceIntegralVariableValuePostprocessor::computeQpIntegral()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_fv)
  {
    /// Get FaceInfo from the mesh
    const FaceInfo * const fi = _mesh.faceInfo(_current_elem, _current_side);
    mooseAssert(fi, "We should have a face info");
    const Elem * neighbor = _current_elem->neighbor_ptr(_current_side);

    /// Get primary variable on the interface
    const Real u = MetaPhysicL::raw_value(_fv_variable->getInternalFaceValue(neighbor, *fi, _u[_qp]));

    /// Get secondary variable on the interface
    const Real u_neighbor = MetaPhysicL::raw_value(dynamic_cast<const MooseVariableFV<Real> *>(
        getFieldVar("neighbor_variable", 0))->getInternalFaceValue(neighbor,
        *fi, _u_neighbor[_qp]));

    return InterfaceValueTools::getQuantity(_interface_value_type, u, u_neighbor);
  }
  else
#endif
    return InterfaceValueTools::getQuantity(_interface_value_type, _u[_qp], _u_neighbor[_qp]);
}
