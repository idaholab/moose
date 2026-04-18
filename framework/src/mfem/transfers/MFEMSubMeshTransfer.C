//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSubMeshTransfer.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", SubMeshTransfer);

namespace Moose::MFEM
{
InputParameters
SubMeshTransfer::validParams()
{
  InputParameters params = ExecutedObject::validParams();
  params.registerBase("Moose::MFEM::SubMeshTransfer");
  params.addClassDescription(
      "Class to transfer MFEM variable data to or from a restricted copy of "
      "the variable defined on "
      " a subspace of an Moose::MFEM::Mesh, represented as an Moose::MFEM::SubMesh.");
  ExecutedObject::addRequiredDependencyParam<VariableName>(
      params,
      "from_variable",
      "MFEM variable to transfer data from. Can be defined on either the parent mesh or a "
      "submesh of it.");
  params.addRequiredParam<VariableName>("to_variable",
                                        "MFEM variable to transfer data into. Can be defined on "
                                        "either the parent mesh or a submesh of it.");
  return params;
}

SubMeshTransfer::SubMeshTransfer(const InputParameters & parameters)
  : ExecutedObject(parameters),
    _source_var_name(getParam<VariableName>("from_variable")),
    _source_var(*getMFEMProblem().getGridFunction(_source_var_name)),
    _result_var_name(getParam<VariableName>("to_variable")),
    _result_var(*getMFEMProblem().getGridFunction(_result_var_name))
{
}

std::optional<std::string>
SubMeshTransfer::suppliedVariableName() const
{
  return _result_var_name;
}

void
SubMeshTransfer::execute()
{
  mfem::ParSubMesh::Transfer(_source_var, _result_var);
}

} // namespace Moose::MFEM
#endif
