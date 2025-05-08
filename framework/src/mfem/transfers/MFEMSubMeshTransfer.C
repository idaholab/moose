#ifdef MFEM_ENABLED

#include "MFEMSubMeshTransfer.h"
#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

registerMooseObject("MooseApp", MFEMSubMeshTransfer);

InputParameters
MFEMSubMeshTransfer::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMSubMeshTransfer");
  params.addClassDescription(
      "Class for tranferring data for corresponding MFEM variables to the problem (`mfem::ParGridFunction`s).");
  params.addRequiredParam<VariableName>("from_variable",
        "MFEM variable to transfer data from. Can be defined on either the parent mesh or a submesh of it.");
  params.addRequiredParam<VariableName>("to_variable",
          "MFEM variable to transfer data into. Can be defined on either the parent mesh or a submesh of it.");        
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
