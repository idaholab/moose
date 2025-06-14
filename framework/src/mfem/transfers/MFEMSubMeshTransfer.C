//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMSubMeshTransfer.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMSubMeshTransfer);

InputParameters
MFEMSubMeshTransfer::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMSubMeshTransfer");
  params.addClassDescription("Class to transfer MFEM variable data to or from a restricted copy of "
                             "the variable defined on an "
                             " a subspace of an MFEMMesh, represented as an MFEMSubMesh.");
  params.addRequiredParam<VariableName>("from_variable",
                                        "MFEM variable to transfer data from. Can be defined on "
                                        "either the parent mesh or a submesh of it.");
  params.addRequiredParam<VariableName>("to_variable",
                                        "MFEM variable to transfer data into. Can be defined on "
                                        "either the parent mesh or a submesh of it.");
  return params;
}

MFEMSubMeshTransfer::MFEMSubMeshTransfer(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    _source_var_name(getParam<VariableName>("from_variable")),
    _source_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_source_var_name)),
    _result_var_name(getParam<VariableName>("to_variable")),
    _result_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_result_var_name))
{
}

void
MFEMSubMeshTransfer::execute()
{
  mfem::ParSubMesh::Transfer(_source_var, _result_var);
}

#endif
