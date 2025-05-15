#ifdef MFEM_ENABLED

#include "MFEMSubMeshBase.h"
#include "MFEMProblem.h"

InputParameters
MFEMSubMeshBase::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMSubMesh");
  return params;
}

MFEMSubMeshBase::MFEMSubMeshBase(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters)
{
}

#endif
