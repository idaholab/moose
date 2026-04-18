//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVariable.h"
#include "MFEMProblem.h"
#include "MooseVariableBase.h"

registerMooseMFEMObject("MooseApp", Variable);

namespace Moose::MFEM
{
InputParameters
Variable::validParams()
{
  InputParameters params = Object::validParams();
  // Create user-facing 'boundary' input for restricting inheriting object to boundaries.
  params.addRequiredParam<Moose::MFEM::FESpaceName>(
      "fespace", "The finite element space this variable is defined on.");
  // Require moose variable parameters (not used!)
  params += MooseVariableBase::validParams();
  params.addClassDescription(
      "Class for adding MFEM variables to the problem (`mfem::ParGridFunction`s).");
  params.registerBase("MooseVariableBase");
  params.registerSystemAttributeName("MooseVariableBase");
  params.addParam<VariableName>(
      "time_derivative",
      "Optional name to assign to the time derivative of the variable in transient problems.");
  return params;
}

Variable::Variable(const InputParameters & parameters)
  : Object(parameters),
    _fespace(getMFEMProblem().getMFEMObject<FESpace>(
        "Moose::MFEM::FESpace", getParam<Moose::MFEM::FESpaceName>("fespace"))),
    _gridfunction(buildGridFunction()),
    _time_derivative_name(
        isParamValid("time_derivative")
            ? getParam<VariableName>("time_derivative")
            : VariableName(
                  getMFEMProblem().getProblemData().time_derivative_map.createTimeDerivativeName(
                      name())))
{
  *_gridfunction = 0.0;
}

const std::shared_ptr<mfem::ParGridFunction>
Variable::buildGridFunction()
{
  return std::make_shared<mfem::ParGridFunction>(_fespace.getFESpace().get());
}

} // namespace Moose::MFEM
#endif
