#ifdef MFEM_ENABLED

#include "MFEMSubMesh.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMSubMesh);

InputParameters
MFEMSubMesh::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("MFEMSubMesh");
  params.addParam<std::vector<SubdomainName>>("block",
    {},
    "The list of blocks (ids) comprising this SubMesh");  
  MooseEnum ordering("NODES VDIM", "VDIM", false);
  params.addParam<MooseEnum>("ordering", ordering, "Ordering style to use for vector DoFs.");
  params.addParam<int>("vdim", 1, "The number of degrees of freedom per basis function.");
  return params;
}

MFEMSubMesh::MFEMSubMesh(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
  _subdomain_names(getParam<std::vector<SubdomainName>>("block")),
  _subdomain_attributes(_subdomain_names.size())
{
  for (unsigned int i = 0; i < _subdomain_names.size(); ++i)
  {
    _subdomain_attributes[i] = std::stoi(_subdomain_names[i]);
  }
}

void
MFEMSubMesh::buildSubMesh()
{
  _submesh = std::make_shared<mfem::ParSubMesh>(
  mfem::ParSubMesh::CreateFromDomain(getMFEMProblem().mesh().getMFEMParMesh(), getSubdomains()));
}

#endif
