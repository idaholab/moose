#ifdef MFEM_ENABLED

#include "MFEMDomainSubMesh.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDomainSubMesh);

InputParameters
MFEMDomainSubMesh::validParams()
{
  InputParameters params = MFEMSubMeshBase::validParams();
  params.addParam<std::vector<SubdomainName>>(
      "block", {}, "The list of blocks (ids) comprising this SubMesh");
  return params;
}

MFEMDomainSubMesh::MFEMDomainSubMesh(const InputParameters & parameters)
  : MFEMSubMeshBase(parameters),
    _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
    _subdomain_attributes(_subdomain_names.size())
{
  for (unsigned int i = 0; i < _subdomain_names.size(); ++i)
  {
    _subdomain_attributes[i] = std::stoi(_subdomain_names[i]);
  }
}

void
MFEMDomainSubMesh::buildSubMesh()
{
  _submesh = std::make_shared<mfem::ParSubMesh>(mfem::ParSubMesh::CreateFromDomain(
      getMFEMProblem().mesh().getMFEMParMesh(), getSubdomains()));
}

#endif
