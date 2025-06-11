#ifdef MFEM_ENABLED

#include "MFEMSubMesh.h"
#include "MFEMProblem.h"

InputParameters
MFEMSubMesh::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMSubMesh");
  return params;
}

MFEMSubMesh::MFEMSubMesh(const InputParameters & parameters) : MFEMGeneralUserObject(parameters) {}

#endif
