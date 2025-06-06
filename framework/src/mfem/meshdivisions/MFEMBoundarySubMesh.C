#ifdef MFEM_ENABLED

#include "MFEMBoundarySubMesh.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMBoundarySubMesh);

InputParameters
MFEMBoundarySubMesh::validParams()
{
  InputParameters params = MFEMSubMesh::validParams();
  params.addParam<std::vector<BoundaryName>>(
      "boundary",
      {"-1"},
      "The list of boundaries (ids) from the mesh where this boundary condition applies. "
      "Defaults to applying BC on all boundaries.");
  return params;
}

MFEMBoundarySubMesh::MFEMBoundarySubMesh(const InputParameters & parameters)
  : MFEMSubMesh(parameters),
    _boundary_names(getParam<std::vector<BoundaryName>>("boundary")),
    _bdr_attributes(_boundary_names.size())
{
  for (const auto i : index_range(_boundary_names))
    _bdr_attributes[i] = std::stoi(_boundary_names[i]);
}

void
MFEMBoundarySubMesh::buildSubMesh()
{
  _submesh = std::make_shared<mfem::ParSubMesh>(mfem::ParSubMesh::CreateFromBoundary(
      getMFEMProblem().mesh().getMFEMParMesh(), getBoundaries()));
}

#endif
