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
  params.addParam<Moose::MFEM::FESpaceName>(
      "fespace", "The finite element space this variable is defined on.");
  params.addParam<std::string>(
      "fespace_hierarchy",
      "Name of a FESpaceHierarchy; the variable lives on its finest level. "
      "Mutually exclusive with 'fespace'.");
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
    _time_derivative_name(
        isParamValid("time_derivative")
            ? getParam<VariableName>("time_derivative")
            : VariableName(
                  getMFEMProblem().getProblemData().time_derivative_map.createTimeDerivativeName(
                      name())))
{
  const bool has_fespace = isParamSetByUser("fespace");
  const bool has_hierarchy = isParamSetByUser("fespace_hierarchy");

  if (has_fespace == has_hierarchy)
    mooseError("Variable '",
               name(),
               "': exactly one of 'fespace' or 'fespace_hierarchy' must be provided.");

  if (has_fespace)
  {
    _fespace_ptr = &getMFEMProblem().getMFEMObject<FESpace>(
        "Moose::MFEM::FESpace", getParam<Moose::MFEM::FESpaceName>("fespace"));
    _par_fespace = _fespace_ptr->getFESpace();
  }
  else
  {
    const auto & hierarchy_name = getParam<std::string>("fespace_hierarchy");
    _par_fespace = getMFEMProblem().getProblemData().fespaces.GetShared(hierarchy_name);
  }

  _gridfunction = buildGridFunction();
  *_gridfunction = 0.0;
}

bool
Variable::isScalar() const
{
  if (_fespace_ptr)
    return _fespace_ptr->isScalar();
  return _par_fespace->GetVDim() == 1;
}

const std::shared_ptr<mfem::ParGridFunction>
Variable::buildGridFunction()
{
  return std::make_shared<mfem::ParGridFunction>(_par_fespace.get());
}

} // namespace Moose::MFEM
#endif
